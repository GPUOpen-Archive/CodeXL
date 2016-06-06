//==============================================================================
// Copyright (c) 2012-2015 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools
/// \file
/// \brief  Definition of data information for Debug information
//==============================================================================
#ifndef DBGINFODATA_H_
#define DBGINFODATA_H_

/// ----------------------------------------------------------------------
/// \note Debug Information - Data Information                         ///
/// The structures defined in this file are created to hold debug      ///
/// information, describing code scopes and the variables contained in ///
/// them.                                                              ///
/// A code scope is an area of the "byte code" ranging from an entire  ///
/// compilation unit (preprocessed file) down to the code scopes       ///
/// defined in the source (e.g. { } blocks in C).                      ///
/// Code scopes have a minimal and maximal address, and are arranged   ///
/// in a hierarchical tree, such that a child scope never has a range  ///
/// exceeding its parent's range.                                      ///
/// Sibling scope ranges must also never intersect.                    ///
/// Note that an implementation may contain only one flat scope.       ///
/// Each code scope may also contain variables, which are defined only ///
/// inside it - and are not available at binary locations outside the  ///
/// scope's range.                                                     ///
/// A variable contains all the information needed to acquire the      ///
/// data from the debugger implementation, as well as all other info   ///
/// that is displayed with the variable (such as its name and typename)///
/// ----------------------------------------------------------------------

// STL:
#include <set>
#include <vector>
#include <string>
#include <string.h> // ::memcpy()

// Local:
#include <DbgInfoDefinitions.h>
#include <DbgInfoUtils.h>
#include <FacilitiesInterface.h>

namespace HwDbg
{
/// -----------------------------------------------------------------------------------------------
/// \struct VariableInfo
/// \brief Description:  Holds the information regarding a variable
/// \tparam typename AddrType: Used to hold the upper and lower ranges of the variable
/// \tparam  typename VarLocationType: How the variable location is expressed
/// -----------------------------------------------------------------------------------------------
template<typename AddrType, typename VarLocationType> struct VariableInfo
{
    /// Abstract variable information
    typedef VariableInfo<AddrType, VarLocationType> FullVariableInfo;
    typedef bool(*VariableMatchingFunc)(const FullVariableInfo& var, const void* matchData, const FullVariableInfo*& pFoundMember);

public:
    /// \brief contains the value of the variable,
    /// if const - The value is an array of <a>unsigned char</a>
    /// if variable - The value is contained in a \a VarLocationType
    union VarValueUnion
    {
        VarLocationType m_varValueLocation; ///< In case variable, this is the var location type
        unsigned char* m_varConstantValue; ///< In case constant, this is the buffer allocated to the const
    };

    /// Default Constructor
    VariableInfo();
    /// Copy Constructor
    VariableInfo(const FullVariableInfo& other);
    /// Destructor
    virtual ~VariableInfo();
    /// An accessor used to set the Variable Value Type to \a constant - receives the buffer size and the value to set
    void SetConstantValue(HwDbgUInt64 bufferSize, unsigned char* pDataBuffer);
    /// Releases the allocated buffer and resets the Variable Value Type to \a variable
    void ClearConstantValue();
    /// Operator overload
    VariableInfo& operator=(const VariableInfo& other);
    /// Accessor to get the Constant attribute:
    bool IsConst() const { return m_isConst; };
    /// Looks for a member variable by name and returns the variable as an out parameter
    bool CanMatchMemberName(const std::string& memberFullName, const FullVariableInfo*& pFoundMember) const;

    /// Members
    std::string m_varName;                      ///< Name of the variable
    std::string m_typeName;                     ///< Type of variable
    HwDbgUInt64 m_varSize;                      ///< Size of the variable
    unsigned int m_varEncoding;                 ///< Encoding
    VarValueUnion m_varValue;                   ///< The value contained in the var - either an byte array or a VarLocationType
    HwDbgInfo_indirection m_varIndirection;     ///< The indirection Type
    unsigned int m_varIndirectionDetail;        ///< Additional data dependent on the indirection type - Pointer address space, array size, etc.
    std::vector<FullVariableInfo> m_varMembers; ///< A vector of member variables
    AddrType m_lowVariablePC;                   ///< The low end of the program counter range
    AddrType m_highVariablePC;                  ///< The high end of the program counter range
    bool m_isParam;                             ///< Var or Param
    bool m_isOutParam;                          ///< is hsa output Parameter
    unsigned int m_brigOffset;                  ///< offset used in isa dwarf

private:
    bool m_isConst; ///< Const or Var / Param, can only be set via SetConstantValue() and ClearConstantValue()
};

/// -----------------------------------------------------------------------------------------------
/// \struct CodeScope
/// \brief Description:  A structure containing debug info which can also contain other code scopes and variables
/// The top level compilation unit is a CodeScope
/// \tparam AddrType: The Address Type
/// \tparam LineType: The LineType
/// \tparam VarLocationType: The Variable Location Type
/// -----------------------------------------------------------------------------------------------
template<typename AddrType, typename LineType, typename VarLocationType> struct CodeScope
{
public:
    /// An Abstract CodeScope
    typedef CodeScope<AddrType, LineType, VarLocationType> FullCodeScope;
    /// An Abstract VariableInfo
    typedef VariableInfo<AddrType, VarLocationType> FullVariableInfo;
    /// The matching function:
    typedef typename FullVariableInfo::VariableMatchingFunc VarMatchFunc;

public:
    /// The scope type - The top level is a compilation unit
    enum ScopeType
    {
        DID_SCT_COMPILATION_UNIT,
        DID_SCT_GLOBAL_SCOPE,
        DID_SCT_FUNCTION,
        DID_SCT_INLINED_FUNCTION,
        DID_SCT_CODE_SCOPE,
        DID_SCT_HSA_ARGUMENT_SCOPE
    };

    /// -----------------------------------------------------------------------------------------------
    /// \struct AddressRange
    /// \brief Description:  Inner struct containing a range of addresses for a code scope
    /// -----------------------------------------------------------------------------------------------
    struct AddressRange
    {
        /// Default Constructor
        AddressRange() {};
        /// Constructor
        AddressRange(AddrType minAddr, AddrType maxAddr) : m_minAddr(minAddr), m_maxAddr(maxAddr) {};
        AddrType m_minAddr; ///< The lower limit
        AddrType m_maxAddr; ///< The upper limit
    };

    /// -----------------------------------------------------------------------------------------------
    /// \struct InlineInformation
    /// \brief Description:  Contains the location of the inlined information - only relevant if scope type is DID_SCT_INLINED_FUNCTION
    /// -----------------------------------------------------------------------------------------------
    struct InlineInformation
    {
        LineType m_inlinedAt; ///< The line from the inlined function was invoked
    };

public:
    /// Constructor
    CodeScope();
    /// Destructor
    virtual ~CodeScope();
    /// Find the innermost CodeScope which contains addr
    const FullCodeScope* FindSmallestScopeContainingAddress(const AddrType& addr) const;
    /// Find the innermost scope containing an address and variable name, and return that variable
    const FullCodeScope* FindClosestScopeContainingVariable(AddrType startAddr, VarMatchFunc pfnMatch, const void* pMatchData, FullVariableInfo& o_variableInfo) const;
    /// Get the stack depth of an address
    int GetStackDepth(const AddrType& addr) const;
    /// Return the lowest address in all the ranges in the scope
    bool GetLowestAddressInScope(AddrType& o_address) const;
    /// Return the highest address in all the ranges in the scope
    bool GetHighestAddressInScope(AddrType& o_address) const;
    /// Put each address in the innermost CodeScope containing it - needs to receive the topmost scope
    bool MapAddressesToCodeScopes(const std::vector<AddrType>& addresses);
    /// Makes sure that variable ranges do not overlap
    void IntersectVariablesInScope();

    /// Members:
    std::string m_scopeName;                           ///< The name of the scope (e.g. function name, class name, etc).
    ScopeType m_scopeType;                          ///< The type of scope.
    VarLocationType* m_pFrameBase;                  ///< The base value for fbreg variables.
    FullCodeScope* m_pParentScope;                  ///< Pointer to parent scope.
    std::vector<AddressRange> m_scopeAddressRanges;    ///< Range of addresses for the scope - can be fragmented so we need more than one.
    bool m_scopeHasNonTrivialAddressRanges;         ///< true iff m_scopeAddressRanges is non-trivial
    InlineInformation m_inlineInfo;                 ///< Inline information - optional, only for inline functions.
    std::set<AddrType> m_addressCache;              ///< Cache of addresses inside the scope, used for "step over" and "step out" operations
    std::vector<FullCodeScope*> m_children;         ///< Child scopes.
    std::vector<FullVariableInfo*> m_scopeVars;     ///< Variables defined in this scope
    bool m_isKernel;                                ///< is hsa kernel
    VarLocationType* m_pWorkitemOffset;             ///< work item offset value location

private:
    /// Internal function which puts each address in the innermost CodeScope containing it - Recursive
    bool InternalMapAddressesToCodeScopes(const std::vector<AddrType>& addresses, std::map<AddrType, FullCodeScope*>& addrScopeMap);
    /// Get the subset of addresses which are in range of the current scope and return them as an out parameter
    bool GetAddressesInRange(const std::vector<AddrType>& addresses, std::vector<AddrType>& o_AddressesInRange) const;
    /// Check whether the specified address is in the code scope
    bool IsAddressInCodeScope(const AddrType& addr) const;
    /// Private Copy constructor - Disallow copying
    CodeScope(const FullCodeScope& other);
    /// Private assignment operator- Disallow copying
    CodeScope& operator=(const FullCodeScope& other);
};

/// --------------------------------------------------------------------------
/// VariableInfo
/// \brief Description: Need a default constructor to use containers
/// --------------------------------------------------------------------------
template<typename AddrType, typename VarLocationType>
VariableInfo<AddrType, VarLocationType>::VariableInfo()
    : m_varName(),
      m_typeName(),
      m_varSize(0),
      m_varEncoding(0),
      m_varIndirection(HWDBGINFO_VIND_DIRECT),
      m_varIndirectionDetail(0),
      m_varMembers(),
      m_lowVariablePC(0),
      m_highVariablePC(0),
      m_isParam(false),
      m_isOutParam(false),
      m_brigOffset(0),
      m_isConst(false)
{
}

/// -----------------------------------------------------------------------------------------------
/// VariableInfo
/// \brief Description: Copy constructor
/// \param[in]          other - operand
/// -----------------------------------------------------------------------------------------------
template<typename AddrType, typename VarLocationType>
VariableInfo<AddrType, VarLocationType>::VariableInfo(const FullVariableInfo& other)
{
    operator=(other);
};

/// -----------------------------------------------------------------------------------------------
/// ~VariableInfo
/// \brief Description: Destructor
/// -----------------------------------------------------------------------------------------------
template<typename AddrType, typename VarLocationType>
VariableInfo<AddrType, VarLocationType>::~VariableInfo()
{
    // Constant value is dynamically allocated and needs to be deleted:
    ClearConstantValue();
};

/// -----------------------------------------------------------------------------------------------
/// SetConstantValue
/// \brief Description: Allocates a buffer dynamically and copies the contents of pDataBuffer to it
/// \param[in]          bufferSize - The size of the allocation
/// \param[in]          pDataBuffer - The buffer from which to copy
/// -----------------------------------------------------------------------------------------------
template<typename AddrType, typename VarLocationType>
void VariableInfo<AddrType, VarLocationType>::SetConstantValue(HwDbgUInt64 bufferSize, unsigned char* pDataBuffer)
{
    ClearConstantValue();
    m_varValue.m_varConstantValue = new unsigned char[static_cast<size_t>(bufferSize)];
    memcpy(m_varValue.m_varConstantValue , pDataBuffer, static_cast<size_t>(bufferSize));
    m_isConst = true;
    m_isParam = false;
};

/// -----------------------------------------------------------------------------------------------
/// ClearConstantValue
/// \brief Description: Cleans up the allocation
/// -----------------------------------------------------------------------------------------------
template<typename AddrType, typename VarLocationType>
void VariableInfo<AddrType, VarLocationType>::ClearConstantValue()
{
    if (m_isConst)
    {
        delete[] m_varValue.m_varConstantValue;
        m_varValue.m_varConstantValue = nullptr;
    }
};

/// -----------------------------------------------------------------------------------------------
/// operator=
/// \brief Description: Assignment operator
/// \param[in]          other - Operand
/// \return copy
/// -----------------------------------------------------------------------------------------------
template<typename AddrType, typename VarLocationType>
VariableInfo<AddrType, VarLocationType>& VariableInfo<AddrType, VarLocationType>::operator=(const FullVariableInfo& other)
{
    m_varName = other.m_varName;
    m_typeName = other.m_typeName;
    m_varSize = other.m_varSize;
    m_varEncoding = other.m_varEncoding;
    m_isConst = other.m_isConst;

    // Const:
    if (m_isConst)
    {
        SetConstantValue(other.m_varSize, other.m_varValue.m_varConstantValue);
    }
    // Variable or Parameter:
    else
    {
        m_varValue.m_varValueLocation = other.m_varValue.m_varValueLocation;
    }

    m_varIndirection = other.m_varIndirection;
    m_varIndirectionDetail = other.m_varIndirectionDetail; // Pointer address space, array size, etc.

    m_varMembers.clear();
    int numOfMembers = (int)other.m_varMembers.size();

    for (int i = 0; i < numOfMembers; i++)
    {
        // Shallow copy of member variables
        m_varMembers.push_back(other.m_varMembers[i]);
    }

    m_lowVariablePC = other.m_lowVariablePC;
    m_highVariablePC = other.m_highVariablePC;
    m_isParam = other.m_isParam;
    m_isOutParam = other.m_isOutParam;
    m_brigOffset = other.m_brigOffset;

    return *this;
}

/// -----------------------------------------------------------------------------------------------
/// CanMatchMemberName
/// \brief Description: Returns true iff memberFullName describes a variable or member name that this variable can match.
/// e.g. if the string is "a.b.c" then this variable must have the name a, and it must have a member called b of a class which has a member called c.
/// \param[in]          memberFullName - The full name of the variable to be found
/// \param[out]          o_pFoundMember - pointer to the found variable
/// \return True : The variable has been found
/// \return False: Otherwise
/// -----------------------------------------------------------------------------------------------
template<typename AddrType, typename VarLocationType>
bool VariableInfo<AddrType, VarLocationType>::CanMatchMemberName(const std::string& memberFullName, const FullVariableInfo*& o_pFoundMember) const
{
    // Tokenize the name:
    bool retVal = false;
    std::string varName = m_varName;

    // Tokenize the string:
    static const char* delimiters = ".";
    size_t strLen = memberFullName.length();
    char* pTokBuffer = new char[strLen + 1];
    ::memcpy(pTokBuffer, memberFullName.c_str(), strLen);
    pTokBuffer[strLen] = (char)0;

    char* pCurrentTok = strtok(pTokBuffer, delimiters);

    if (nullptr != pCurrentTok)
    {
        std::string currentToken = pCurrentTok;;

        // The first token should be the variable name:
        if (varName == currentToken)
        {
            // The base variable was found, start looking in its member list:
            retVal = true;
            const VariableInfo<AddrType, VarLocationType>* pCurrentMemberDetails = this;

            while (nullptr != (pCurrentTok = strtok(nullptr, delimiters)))
            {
                currentToken = pCurrentTok;

                // Check if this is a member of the current struct:
                int numberOfSubitems = (int)pCurrentMemberDetails->m_varMembers.size();
                bool foundSubitem = false;

                for (int i = 0; i < numberOfSubitems; i++)
                {
                    // Check if this is the requested subitem:
                    const VariableInfo<AddrType, VarLocationType>& currentSubitem = pCurrentMemberDetails->m_varMembers[i];

                    if (currentSubitem.m_varName == currentToken)
                    {
                        // Iterate into it:
                        pCurrentMemberDetails = &currentSubitem;

                        // Stop looking here:
                        foundSubitem = true;
                        break;
                    }
                }

                // If this member doesn't exist:
                if (!foundSubitem)
                {
                    // Stop looking for this value, the expression is not valid:
                    retVal = false;
                    break;
                }
            }

            // Return the found member:
            if (retVal)
            {
                o_pFoundMember = pCurrentMemberDetails;
            }
        }
    }

    delete[] pTokBuffer;

    return retVal;
}

/// -----------------------------------------------------------------------------------------------
/// CodeScope
/// \brief Description: Default constructor
/// -----------------------------------------------------------------------------------------------
template<typename AddrType, typename LineType, typename VarLocationType>
CodeScope<AddrType, LineType, VarLocationType>::CodeScope()
    : m_scopeType(DID_SCT_COMPILATION_UNIT), m_pFrameBase(nullptr), m_pParentScope(nullptr), m_scopeHasNonTrivialAddressRanges(false), m_isKernel(false), m_pWorkitemOffset(nullptr)
{
};

/// -----------------------------------------------------------------------------------------------
/// ~CodeScope
/// \brief Description: Destructor
/// -----------------------------------------------------------------------------------------------
template<typename AddrType, typename LineType, typename VarLocationType>
CodeScope<AddrType, LineType, VarLocationType>::~CodeScope()
{
    if (m_pFrameBase != nullptr)
    {
        delete m_pFrameBase;
        m_pFrameBase = nullptr;
    }

    // Delete the children:
    size_t numberOfChildren = m_children.size();

    for (size_t i = 0; i < numberOfChildren; i++)
    {
        delete m_children[i];
        m_children[i] = nullptr;
    }

    m_children.clear();

    // Delete the variables:
    size_t numberOfVars = m_scopeVars.size();

    for (size_t i = 0; i < numberOfVars; i++)
    {
        delete m_scopeVars[i];
        m_scopeVars[i] = nullptr;
    }

    m_scopeVars.clear();
}

/// -----------------------------------------------------------------------------------------------
/// FindSmallestScopeContainingAddress
/// \brief Description: Find the innermost child scope containing the address - we assume that sibling scopes do not intersect.
/// \param[in]          addr - the address to find
/// \return A pointer to the codescope
/// -----------------------------------------------------------------------------------------------
template<typename AddrType, typename LineType, typename VarLocationType>
const CodeScope<AddrType, LineType, VarLocationType>* CodeScope<AddrType, LineType, VarLocationType>::FindSmallestScopeContainingAddress(const AddrType& addr) const
{
    const FullCodeScope* pRetVal = nullptr;
    const FullCodeScope* pTmpVal = nullptr;

    if (IsAddressInCodeScope(addr))
    {
        // At the very least, the address is contained in this scope:
        pRetVal = this;

        // Iterate over child scopes:
        int numberOfScopes = (int)m_children.size();

        // Prefer Scopes that have specific ranges over scopes that have just the default (0, infinity) range:
        for (int i = 0; i < numberOfScopes; i++)
        {
            const FullCodeScope* pCurrentScope = m_children[i];

            if (nullptr != pCurrentScope)
            {
                if (pCurrentScope->m_scopeHasNonTrivialAddressRanges)
                {
                    // Recursive call - to find the 'deepest' child scope containing the address:
                    pTmpVal = pCurrentScope->FindSmallestScopeContainingAddress(addr);

                    if (nullptr != pTmpVal)
                    {
                        pRetVal = pTmpVal;
                        break;
                    }
                }
            }
        }

        if (this == pRetVal)
        {
            for (int i = 0; i < numberOfScopes; i++)
            {
                const FullCodeScope* pCurrentScope = m_children[i];

                if (nullptr != pCurrentScope)
                {
                    if (!pCurrentScope->m_scopeHasNonTrivialAddressRanges)
                    {
                        // Recursive call - to find the 'deepest' child scope containing the address:
                        pTmpVal = pCurrentScope->FindSmallestScopeContainingAddress(addr);

                        if (nullptr != pTmpVal)
                        {
                            pRetVal = pTmpVal;
                            break;
                        }
                    }
                }
            }
        }
    }

    // Return the selected code scope or nullptr if none is found:
    return pRetVal;
}

/// -----------------------------------------------------------------------------------------------
/// GetStackDepth
/// \brief Description: Return the stack depth of the chosen address
/// \param[in]          addr - the address to look at
/// \return the stack depth of the address
/// -----------------------------------------------------------------------------------------------
template<typename AddrType, typename LineType, typename VarLocationType>
int CodeScope<AddrType, LineType, VarLocationType>::GetStackDepth(const AddrType& addr) const
{
    int retVal = 0;
    const FullCodeScope* pBottomScope = FindSmallestScopeContainingAddress(addr);
    const FullCodeScope* pTempScope = pBottomScope;

    if (pTempScope != nullptr)
    {
        // End condition:
        while (pTempScope != this)
        {
            // Increment stack depth:
            ++retVal;
            // Go up one level:
            pTempScope = pTempScope->m_pParentScope;
        }
    }

    return retVal;
}

/// -----------------------------------------------------------------------------------------------
/// GetLowestAddressInScope
/// \brief Description: Returns the lowest address in the scope
/// \param[out]         o_address - the address
/// \return True : An address has been found and returned
/// \return False: No addresses found
/// -----------------------------------------------------------------------------------------------
template<typename AddrType, typename LineType, typename VarLocationType>
bool CodeScope<AddrType, LineType, VarLocationType>::GetLowestAddressInScope(AddrType& o_address) const
{
    bool retVal = false;

    int numberOfRanges = (int)m_scopeAddressRanges.size();

    for (int i = 0; i < numberOfRanges; i++)
    {
        // If this is the first address range, or if it is the minimum:
        if ((0 == i) || (m_scopeAddressRanges[i].m_minAddr < o_address))
        {
            // Update the return values:
            retVal = true;
            o_address = m_scopeAddressRanges[i].m_minAddr;
        }
    }

    return retVal;
}

/// -----------------------------------------------------------------------------------------------
/// GetHighestAddressInScope
/// \brief Description: Returns the top address range in the scope
/// \param[out]         o_address - the highest address in the range
/// \return True : An address has been found and returned
/// \return False: No addresses found
/// -----------------------------------------------------------------------------------------------
template<typename AddrType, typename LineType, typename VarLocationType>
bool CodeScope<AddrType, LineType, VarLocationType>::GetHighestAddressInScope(AddrType& o_address) const
{
    bool retVal = false;

    int numberOfRanges = (int)m_scopeAddressRanges.size();

    for (int i = 0; i < numberOfRanges; i++)
    {
        // If this is the first address range, or if it is the minimum:
        if ((0 == i) || (m_scopeAddressRanges[i].m_maxAddr > o_address))
        {
            // Update the return values:
            retVal = true;
            o_address = m_scopeAddressRanges[i].m_maxAddr;
        }
    }

    return retVal;
}

/// -----------------------------------------------------------------------------------------------
/// IsAddressInCodeScope
/// \brief Description: Returns whether an address is contained in the scope
/// \param[in]          addr - the address
/// \return True : Address is in CodeScope
/// \return False: Otherwise
/// -----------------------------------------------------------------------------------------------
template<typename AddrType, typename LineType, typename VarLocationType>
bool CodeScope<AddrType, LineType, VarLocationType>::IsAddressInCodeScope(const AddrType& addr) const
{
    bool retVal = false;

    int numberOfRanges = (int)m_scopeAddressRanges.size();

    for (int i = 0; i < numberOfRanges; i++)
    {
        // NOTE: DWARF defines the ranges as [min, max) - meaning the right condition should be < rather than <=.
        // We allow the border cases for call stack construction.
        // Check that the address is in the range:
        if ((addr >= m_scopeAddressRanges[i].m_minAddr) && (addr <= m_scopeAddressRanges[i].m_maxAddr))
        {
            retVal = true;
            break;
        }
    }

    return retVal;
}

/// -----------------------------------------------------------------------------------------------
/// GetAddressesInRange
/// \brief Description: fills the output vector with the subset of all addresses which are in the scope's range
/// \param[in]          addresses - the addresses to check
/// \param[out]         o_AddressesInRange - the subset of in-range addresses to return
/// \return True : addresses are found
/// \return False: no addresses found
/// -----------------------------------------------------------------------------------------------
template<typename AddrType, typename LineType, typename VarLocationType>
bool CodeScope<AddrType, LineType, VarLocationType>::GetAddressesInRange(const std::vector<AddrType>& addresses, std::vector<AddrType>& o_AddressesInRange) const
{
    bool retVal = false;
    o_AddressesInRange.clear();

    // Iterate over the scope's address ranges:
    int numberOfAddrs = (int)addresses.size();

    for (int i = 0; i < numberOfAddrs; i++)
    {
        const AddrType& currentAddr = addresses[i];

        if (IsAddressInCodeScope(currentAddr))
        {
            o_AddressesInRange.push_back(currentAddr);
        }
    }

    retVal = !o_AddressesInRange.empty();

    return retVal;
}

/// -----------------------------------------------------------------------------------------------
/// MapAddressesToCodeScopes
/// \brief Description: Fills the relevant scopes' address caches with the corresponding addresses
/// \param[in]          addresses - addresses to cache
/// \return True : at least one address was cached successfully
/// \return False: Otherwise
/// -----------------------------------------------------------------------------------------------
template<typename AddrType, typename LineType, typename VarLocationType>
bool CodeScope<AddrType, LineType, VarLocationType>::MapAddressesToCodeScopes(const std::vector<AddrType>& addresses)
{
    bool retVal = false;
    // Only allow this function to be run on the top level scope:
    HWDBG_ASSERT((m_scopeType == FullCodeScope::DID_SCT_COMPILATION_UNIT || m_scopeType == FullCodeScope::DID_SCT_GLOBAL_SCOPE) && m_pParentScope == nullptr);

    if ((m_scopeType == FullCodeScope::DID_SCT_COMPILATION_UNIT || m_scopeType == FullCodeScope::DID_SCT_GLOBAL_SCOPE) && m_pParentScope == nullptr)
    {
        // Initialize map:
        std::map<AddrType, FullCodeScope*> addrScopeMap;
        // Fills the map:
        retVal = InternalMapAddressesToCodeScopes(addresses, addrScopeMap);

        // Fill the caches with the map:
        if (retVal)
        {
            // For each pair of address and code scope in the map, add the address to the code scope's cache:
            for (typename std::map<AddrType, FullCodeScope*>::iterator it = addrScopeMap.begin(); it != addrScopeMap.end(); ++ it)
            {
                AddrType addrType = it->first;
                FullCodeScope* pCodeScope = it->second;
                HWDBG_ASSERT(pCodeScope->m_pParentScope == nullptr || pCodeScope->m_scopeType == FullCodeScope::DID_SCT_INLINED_FUNCTION || pCodeScope->m_scopeType == FullCodeScope::DID_SCT_FUNCTION);
                pCodeScope->m_addressCache.insert(addrType);
            }
        }
    }

    return retVal;
}

/// -----------------------------------------------------------------------------------------------
/// InternalMapAddressesToCodeScopes
/// \brief Description: Internal version of the above function which fills the map
/// \param[in]          addresses - addresses to map
/// \param[out]         o_addrScopeMap - output map of address -> scope
/// \return True : at least one pair mapped
/// \return False: Otherwise
/// -----------------------------------------------------------------------------------------------
template<typename AddrType, typename LineType, typename VarLocationType>
bool CodeScope<AddrType, LineType, VarLocationType>::InternalMapAddressesToCodeScopes(const std::vector<AddrType>& addresses, std::map<AddrType, FullCodeScope*>& o_addrScopeMap)
{
    bool retVal = false;

    // If this scope is the top level, or it is a function, get its memory addresses:
    if (m_pParentScope == nullptr || m_scopeType == FullCodeScope::DID_SCT_INLINED_FUNCTION || m_scopeType == FullCodeScope::DID_SCT_FUNCTION)
    {
        std::vector<AddrType> addressesInRange;
        GetAddressesInRange(addresses, addressesInRange);

        // Fill the map with the Addr->pScope mapping:
        int numberOfAddrs = (int)addressesInRange.size();

        for (int i = 0; i < numberOfAddrs; i++)
        {
            const AddrType& currentAddr = addressesInRange[i];

            if (m_scopeHasNonTrivialAddressRanges)
            {
                o_addrScopeMap[currentAddr] = this;
            }
            else
            {
                // For trivially-ranged scopes, only save addresses that are not already mapped (if a non-trivial sibling appears later, it will overtake
                // these addresses).
                typename std::map<AddrType, FullCodeScope*>::const_iterator findIter = o_addrScopeMap.find(currentAddr);
                typename std::map<AddrType, FullCodeScope*>::const_iterator endIter = o_addrScopeMap.end();

                if (endIter == findIter)
                {
                    o_addrScopeMap[currentAddr] = this;
                }
            }

            retVal = true;
        }
    }

    // For each child scope:
    int numberOfChildren = (int)m_children.size();

    for (int i = 0; i < numberOfChildren; i++)
    {
        FullCodeScope* pCurrentChild = m_children[i];

        if (nullptr != pCurrentChild)
        {
            // Recursive function call to fill map. It is enough that one child succeeds even if we failed:
            retVal = pCurrentChild->InternalMapAddressesToCodeScopes(addresses, o_addrScopeMap) || retVal;
        }
    }

    return retVal;
}

/// -----------------------------------------------------------------------------------------------
/// FindClosestScopeContainingVariable
/// \brief Description: Finds the smallest scope that contains startAddr and defines a variable named varName
/// \note Constants only exist once in a scope, so we do not need to check their ranges
/// \param[in]          startAddr - The address to look in
/// \param[in]          varName - the name of the variable we are looking for
/// \param[out]         o_variableInfo - The variable
/// \return pointer to the closest scope containing the variable
/// -----------------------------------------------------------------------------------------------
template<typename AddrType, typename LineType, typename VarLocationType>
const CodeScope<AddrType, LineType, VarLocationType>* CodeScope<AddrType, LineType, VarLocationType>::FindClosestScopeContainingVariable(AddrType startAddr, VarMatchFunc pfnMatch, const void* pMatchData, FullVariableInfo& o_variableInfo) const
{
    const FullCodeScope* retVal = nullptr;

    // Find the smallest scope that contains startAddr:
    const FullCodeScope* pCurrentScope = FindSmallestScopeContainingAddress(startAddr);

    while (pCurrentScope != nullptr)
    {
        // See if this scope contains a variable named correctly:
        int numberOfVars = (int)pCurrentScope->m_scopeVars.size();
        bool foundVar = false;

        for (int i = 0; i < numberOfVars; ++i)
        {
            // Check if this variable is the correct one:
            const FullVariableInfo* pCurrentVar = pCurrentScope->m_scopeVars[i];
            HWDBG_ASSERT(pCurrentVar != nullptr);

            if (pCurrentVar != nullptr)
            {
                const FullVariableInfo* pFoundMember = nullptr;

                if (pfnMatch(*pCurrentVar, pMatchData, pFoundMember))
                {
                    // If const:
                    if (pCurrentVar->IsConst())
                    {
                        foundVar = true;
                        o_variableInfo = *pFoundMember;
                        break;
                    }
                    // If this is a variable or a parameter:
                    else
                    {
                        // If the address is in the variable's scope:
                        if ((pCurrentVar->m_highVariablePC >= startAddr) && (pCurrentVar->m_lowVariablePC <= startAddr))
                        {
                            // We found the variable!
                            foundVar = true;
                            o_variableInfo = *pFoundMember;
                        }
                    }
                }
            }
        }

        if (!foundVar)
        {
            // Do not traverse function boundaries, unless the address given is exactly at the boundary:
            bool shouldTraverse = (DID_SCT_CODE_SCOPE == pCurrentScope->m_scopeType);

            // This sometimes generates incorrect results when the boundary address is one of the addresses mapped.
            // Uncommenting may result in mis-mapping addresses.
            /*
            if (!shouldTraverse)
            {
                // Get the boundary addresses:
                AddrType minAddr;
                bool rcMin = pCurrentScope->GetLowestAddressInScope(minAddr);

                AddrType maxAddr;
                bool rcMax = pCurrentScope->GetHighestAddressInScope(maxAddr);

                // Check if the start address is one of them:
                shouldTraverse = (rcMin && (startAddr == minAddr)) || (rcMax && (startAddr == maxAddr));
            }
            */

            if (shouldTraverse)
            {
                // Rise up in the hierarchy until you find the variable:
                pCurrentScope = pCurrentScope->m_pParentScope;
            }
            else
            {
                // We could not find the variable in the given scope hierarchy:
                pCurrentScope = nullptr;
                break;
            }
        }
        else // foundVar:
        {
            // This scope is the smallest that contains a matching variable, return it:
            retVal = pCurrentScope;
            break;
        }
    }

    return retVal;
}

/// -----------------------------------------------------------------------------------------------
/// IntersectVariablesInScope
/// \brief Description: Goes over the variables list in the scope, intersecting the ranges of
/// like-named variables that do not have their own high range but use the scope's high range instead
/// in order for the variables not to overlap
/// -----------------------------------------------------------------------------------------------
template<typename AddrType, typename LineType, typename VarLocationType>
void CodeScope<AddrType, LineType, VarLocationType>::IntersectVariablesInScope()
{
    // Iterate all the variables:
    int numberOfVariables = (int)(m_scopeVars.size());

    for (int i = 0; i < numberOfVariables; ++i)
    {
        // Sanity check:
        FullVariableInfo* pCurrentVariable = m_scopeVars[i];
        HWDBG_ASSERT(pCurrentVariable != nullptr);

        if (pCurrentVariable != nullptr)
        {
            // Do not need to intersect Constants:
            if (pCurrentVariable->IsConst())
            {
                continue;
            }

            // Get all the other variables:
            for (int j = (i + 1); j < numberOfVariables; j++)
            {
                // No need to assert twice for the same variable:
                FullVariableInfo* pOtherVariable = m_scopeVars[j];

                if (nullptr != pOtherVariable)
                {
                    // Do not need to intersect Constants:
                    if (pOtherVariable->IsConst())
                    {
                        continue;
                    }

                    // If these two variables have the same name:
                    if (pCurrentVariable->m_varName == pOtherVariable->m_varName)
                    {
                        // Check if the current variable needs to be truncated:
                        AddrType maxAddr = ULLONG_MAX;
                        GetHighestAddressInScope(maxAddr);
                        HWDBG_ASSERT(maxAddr > 0);

                        if (maxAddr > 0)
                        {
                            if (pCurrentVariable->m_highVariablePC >= maxAddr)
                            {
                                if ((pCurrentVariable->m_highVariablePC > pOtherVariable->m_lowVariablePC) &&
                                    (pCurrentVariable->m_lowVariablePC < pOtherVariable->m_lowVariablePC))
                                {
                                    // Truncate it:
                                    pCurrentVariable->m_highVariablePC = pOtherVariable->m_lowVariablePC;
                                    pCurrentVariable->m_highVariablePC--;
                                }
                            }

                            // Check if the other variable needs to be truncated:
                            if (pOtherVariable->m_highVariablePC >= maxAddr)
                            {
                                if ((pOtherVariable->m_highVariablePC > pCurrentVariable->m_lowVariablePC) &&
                                    (pOtherVariable->m_lowVariablePC < pCurrentVariable->m_lowVariablePC))
                                {
                                    // Truncate it:
                                    pOtherVariable->m_highVariablePC = pCurrentVariable->m_lowVariablePC;
                                    pOtherVariable->m_highVariablePC--;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
} // namespace HwDbg

#endif //DBGINFODATA_H_
