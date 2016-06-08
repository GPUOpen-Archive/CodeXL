//==============================================================================
// Copyright (c) 2012-2015 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools
/// \file
/// \brief  Interface definition of Debug Information Consumer
//==============================================================================
#ifndef DBGINFOICONSUMER_H_
#define DBGINFOICONSUMER_H_

/// AMDTBaseTools:
#include <vector>

/// Local:
#include <DbgInfoIConsumer.h>
#include <DbgInfoDefinitions.h>

namespace HwDbg
{

/// -----------------------------------------------------------------------------------------------
/// \class DbgInfoCodeContext
/// \brief Description:  A code context, used to descibe a call stack frame
/// \tparam AddrType: Address representation - In Dwarf this is HwDbgUInt64
/// \tparam LineType: Line Representation - In Dwarf this is a File name and a line number
/// -----------------------------------------------------------------------------------------------
template<typename AddrType, typename LineType>
struct DbgInfoCodeContext
{
public:
    /// Program counter for the stack frame:
    AddrType m_programCounter;
    /// Function start address:
    AddrType m_functionBase;
    /// Module start address:
    AddrType m_moduleBase;
    /// Line number and file (or equivalent):
    LineType m_sourceLocation;
    /// Function name:
    std::string m_functionName;
};

/// -----------------------------------------------------------------------------------------------
/// \class DbgInfoIConsumer
/// \brief Description:  Interface for a consumer contains a definition of a line, an address and a variable location
/// and knows how to answer complex queries about them
/// \tparam AddrType: Address representation - In Dwarf this is HwDbgUInt64
/// \tparam LineType: Line Representation - In Dwarf this is a File name and a line number
/// \tparam VarLocationType: Variable Location Representation - In Dwarf this is a \a DwarfVariableLocation
/// -----------------------------------------------------------------------------------------------
template<typename AddrType, typename LineType, typename VarLocationType>
class DbgInfoIConsumer
{
public:
    /// Some typedefs for ease of use:
    //@{
    // typedef DbgInfoCodeContext<AddrType, LineType> DbgInfoCallStackFrame;
    typedef VariableInfo<AddrType, VarLocationType> ConsumedVariableInfo;
    typedef CodeScope<AddrType, LineType, VarLocationType> ConsumedCodeScope;
    typedef typename ConsumedVariableInfo::VariableMatchingFunc VarMatchFunc;
    //@}

public:
    DbgInfoIConsumer() {};
    virtual ~DbgInfoIConsumer() {};

public:
    /// Retrieve a line from an address, output parameter - Line
    virtual bool GetLineFromAddress(const AddrType& addr, LineType& o_line) const = 0;
    /// Retrieve mapped addresses from line, output parameter - Vector of Addresses
    virtual bool GetAddressesFromLine(const LineType& line, std::vector<AddrType>& o_addresses, bool append, bool firstAddr = false) const = 0;
    /// Returns the nearest mapped line to the specified line, if the line is mapped then it is returned. False if no near enough mapped line exists
    virtual bool GetNearestMappedLine(const LineType& line, LineType& o_nearestMappedLine) const = 0;
    /// Returns the nearest mapped address to the specified address, if the address is mapped then it is returned. False if no near enough mapped address exists
    virtual bool GetNearestMappedAddress(const AddrType& addr, AddrType& o_nearestMappedAddress) const = 0;
    /// Return all mapped addresses
    virtual bool GetMappedAddresses(std::vector<AddrType>& o_addresses) const = 0;
    /// Given a start address, the virtual CallStack ending in that address, The first line is the one mapped to the address, the rest are the locations of the inlined functions containing the address. The stack is appended, do not clear at entry.
    virtual bool GetAddressVirtualCallStack(const AddrType& startAddr, std::vector<DbgInfoCodeContext<AddrType, LineType> >& io_stack) const = 0;
    /// Returns a vector of cached addresses - these are the addresses cached in each scope signifying the innermost scope that they appear in
    virtual bool GetCachedAddresses(const AddrType& startAddr, bool includeCurrentScope, std::vector<AddrType>& o_cachedAddresses) const = 0;
    /// Given an address and a variable name returns the corresponding variable info
    virtual bool GetMatchingVariableInfoInCurrentScope(const AddrType& startAddr, VarMatchFunc pfnMatch, const void* pMatchData, ConsumedVariableInfo& o_variable) const = 0;
    /// Given an address and a variable name returns the frameBase belonging to the scope containing said address and var name
    virtual bool GetFrameBase(AddrType startAddr, const std::string& varName, VarLocationType& o_frameBase) const = 0;
    /// Given a target stack frame depth, returns all the variable names belonging to that depth and all globals, (-1 returns all variables)
    virtual bool ListVariablesFromAddress(const AddrType& addr, int stackFrameDepth, bool finalMembers, std::vector<std::string>& o_variableNames) const = 0;
    /// Gets the stack depth of a given address, counting containing inlined/non inlined functions scopes
    virtual int GetAddressStackDepth(const AddrType& addr) const = 0;

public:
    /// Variable matching function for the name field:
    static bool MatchVariableByName(const ConsumedVariableInfo& var, const void* matchData, const ConsumedVariableInfo*& pFoundMember)
    {
        return (nullptr != matchData) ? (var.CanMatchMemberName(*(const std::string*)matchData, pFoundMember)) : false;
    };

    /// Given an address and a variable name returns the corresponding variable info, using the above (implementation-specific) overload
    bool GetVariableInfoInCurrentScope(const AddrType& startAddr, const std::string& variableName, ConsumedVariableInfo& o_variable) const
    {
        return GetMatchingVariableInfoInCurrentScope(startAddr, MatchVariableByName, (const void*)(&variableName), o_variable);
    };
};
} // namespace HwDbg
#endif // DBGINFOICONSUMER_H_
