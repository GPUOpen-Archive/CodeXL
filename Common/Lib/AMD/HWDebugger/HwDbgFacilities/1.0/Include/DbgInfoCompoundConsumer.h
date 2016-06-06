//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools
/// \file
/// \brief  Definition of the DbgInfoCompoundConsumer
//==============================================================================
#ifndef DBGINFOCOMPOUNDCONSUMER_H_
#define DBGINFOCOMPOUNDCONSUMER_H_

#include <DbgInfoIConsumer.h>

// STL:
#include <string>
#include <vector>

namespace HwDbg
{

///////////////////////////////////////////////////////////////////////////////////////////////////
/// \class DbgInfoCompoundConsumer
/// \brief Description:  Receives pointers to 2 consumers - High/Low and knows to answer joint queries about them
/// We require that the HighAddressType be the same as (or transferable to) LowLineType
/// \brief Note: One needs to supply a helper function which gets a low level location from a high level location (we have an example for dwarf at: <em>DbgInfoDwarfParser::DwarfLocationResolver()</em>)
/// \tparam HAddrType - The high level address type (\a HwDbgUInt64 in Dwarf Source->Brig)
/// \tparam HLineType - The high level line type (<em>DbgInfoLine::FileLocation</em> in Dwarf Source->Brig)
/// \tparam HVarLocationType - The high level variable location type - (<em>DbgInfoDwarfParser::DwarfVariableLocation</em> in Dwarf Source->Brig)
/// \tparam LAddrType - The low level address type (\a HwDbgUInt64 in Dwarf Brig->ISA)
/// \tparam LLineType - The low level line type (\a HwDbgUInt64 in Dwarf Brig->ISA)
/// \tparam LVarLocationType - The low level variable location type - (<em>DbgInfoDwarfParser::DwarfVariableLocation</em> in Dwarf Brig->ISA)
///////////////////////////////////////////////////////////////////////////////////////////////////
template<typename HAddrType, typename HLineType, typename HVarLocationType, typename LAddrType, typename LVarLocationType, typename LLineType = HAddrType>
class DbgInfoCompoundConsumer : public DbgInfoIConsumer<LAddrType, HLineType, LVarLocationType>
{
public:
    /// A high level consumer - Source->Brig
    typedef DbgInfoIConsumer<HAddrType, HLineType, HVarLocationType> HighLvlConsumer;
    /// A high level consumer - Brig->ISA
    typedef DbgInfoIConsumer<LAddrType, LLineType, LVarLocationType> LowLvlConsumer;
    /// Source->ISA consumer interface
    typedef DbgInfoIConsumer<LAddrType, HLineType, LVarLocationType> TwoLvlIConsumer;
    /// Brig variable info
    typedef VariableInfo<HAddrType, HVarLocationType> HighLvlVariableInfo;
    /// ISA variable info
    typedef VariableInfo<LAddrType, LVarLocationType> LowLvlVariableInfo;
    /// Source->ISA call stack frame
    typedef DbgInfoCodeContext<LAddrType, HLineType> TwoLvlCallStackFrame;
    /// Source->Brig call stack frame
    typedef DbgInfoCodeContext<HAddrType, HLineType> HighLvlCallStackFrame;
    /// Brig->ISA call stack frame
    typedef DbgInfoCodeContext<LAddrType, LLineType> LowLvlCallStackFrame;

    /// Function pointer to get a low variable location from a high variable location - must be supplied to compound consumer in order to run \a GetVariableInfoInCurrentScope and \a GetFrameBase:
    /// The last parameter -\a userData, is optional and in the Dwarf case contains a location in the brig file from which to read the variable name
    typedef bool (*LocationResolver)(const HVarLocationType& hVarLocType, const LAddrType& lAddrType, const LowLvlConsumer& lConsumer, LVarLocationType& o_lVarLocationType, void* userData);
    /// Function pointer to convert High-level addresses to Low-Level lines:
    typedef LLineType(*AddressResolver)(const HAddrType& hAddr, void* userData);
    /// Function pointer to convert Low-Level lines to High-level addresses:
    typedef HAddrType(*LineResolver)(const LLineType& hAddr, void* userData);

public:
    /// Default function pointer for location resolver, should be fine to use as long as you do not need to resolve the low level variable location.
    static bool DefaultLocationResolver(const HVarLocationType& hVarLoc, const LAddrType& lAddr, const LowLvlConsumer& lConsumer, LVarLocationType& o_lVarLocation, void* userData)
    {
        // Not implemented for this function:
        HWDBG_ASSERT(false);
        return false;
    }

    /// Default conversion function if LLine = HAddr or the conversion is trivial:
    static LLineType DefaultAddressResolver(const HAddrType& hAddr, void* userData)
    {
        LLineType retVal;

        retVal = hAddr;

        return retVal;
    };

    /// Default conversion function if HAddr = LLine or the conversion is trivial:
    static HAddrType DefaultLineResolver(const LLineType& lLine, void* userData)
    {
        HAddrType retVal;

        retVal = lLine;

        return retVal;
    };

public:
    /// Constructor - need to pass in 2 consumers by reference, optional parameters are the userData and the pLocationResolver function pointer.
    /// If these are not supplied, a default implementation is provided which throws an assertion if used.
    /// This class takes ownership of the consumers
    DbgInfoCompoundConsumer(HighLvlConsumer* pHConsumer, LowLvlConsumer* pLConsumer, LocationResolver pLocationResolver = &DbgInfoCompoundConsumer::DefaultLocationResolver, AddressResolver pAddrResolver = &DbgInfoCompoundConsumer::DefaultAddressResolver, LineResolver pLineResolver = &DbgInfoCompoundConsumer::DefaultLineResolver, void* userData = nullptr, bool firstMappedLLAddr = false);
    /// Destructor
    virtual ~DbgInfoCompoundConsumer();

    /// receives a low level address and return a high level line - if there is no direct mapping it will get a line mapping to the nearest mapped address
    virtual bool GetLineFromAddress(const LAddrType& lAddr, HLineType& o_hLine) const;
    /// receives a high level line and return the list of low level addresses mapped to it or to the nearest low level line that has such a mapping
    virtual bool GetAddressesFromLine(const HLineType& hLine, std::vector<LAddrType>& o_lAddresses, bool append = false, bool firstAddr = false) const;
    /// Gets nearest mapped line
    virtual bool GetNearestMappedLine(const HLineType& hLine, HLineType& o_nearestMappedHLine) const;
    /// Gets nearest mapped address
    virtual bool GetNearestMappedAddress(const LAddrType& lAddr, LAddrType& o_nearestMappedLAddress) const;
    /// Gets all low level addresses currently mapped
    virtual bool GetMappedAddresses(std::vector<LAddrType>& o_lAddresses) const;
    /// This function returns the virtual call stack. All the <hLine,lAddr> pairs go into the o_stack vector and are returned
    virtual bool GetAddressVirtualCallStack(const LAddrType& startLAddr, std::vector<TwoLvlCallStackFrame>& io_stack) const;
    /// Gets the cached high level addresses, and for each of them gets all the the corresponding low level addresses
    virtual bool GetCachedAddresses(const LAddrType& startLAddr, bool includeCurrentScope, std::vector<LAddrType>& o_cachedLAddresses) const;
    /// Returns the low level variable given a high level var name and an low level address.
    virtual bool GetMatchingVariableInfoInCurrentScope(const LAddrType& startLAddr, typename TwoLvlIConsumer::VarMatchFunc pfnMatch, const void* pMatchData, LowLvlVariableInfo& o_variable) const;
    /// Gets the frame base which is a low level variable location
    virtual bool GetFrameBase(LAddrType startLAddr, const std::string& varName, LVarLocationType& o_lFrameBase) const;
    /// Translates scope and gets a list of all high level variables defined in address's scope and all containing scopes
    virtual bool ListVariablesFromAddress(const LAddrType& addr, int stackFrameDepth, bool finalMembers, std::vector<std::string>& o_variableNames) const;
    /// Returns the high level stack depth.
    virtual int GetAddressStackDepth(const LAddrType& lAddr) const;

private:
    /// Recursively fills a low level variable from a high level variable
    void CopyHighToLowVariable(const LAddrType& lAddr, const HighLvlVariableInfo& hVar, LowLvlVariableInfo& o_lVar) const;

private:
    DbgInfoIConsumer<HAddrType, HLineType, HVarLocationType>* m_pHConsumer; ///< Pointer to High Level Consumer - set at constructor
    DbgInfoIConsumer<LAddrType, LLineType, LVarLocationType>* m_pLConsumer; ///< Pointer to Low Level Consumer - set at constructor
    LocationResolver m_pLocationResolver; ///< Function pointer, knows how to return an LVarLocationType given (HVarLocationType, LAddrType,  LowLvlConsumer, void* userData) where the userData can be anything the user needs
    AddressResolver m_pAddrResolver; ///< Function pointer, converts a high-level address (HAddr) to low-level line (LLine)
    LineResolver m_pLineResolver; ///< Function pointer, converts a low-level line (LLine) to high-level address (HAddr)
    void* m_pResolverUserData; ///< User data: used for resolver functions, can contain whatever metadata the user needs, in DWARF case, contains a pointer to the brig variable containing the low level variable name
    bool m_firstMappedLLAddr; ///< True if we use only the first LL address for each HL address / LL line. False if we use all of them.
}; /// class DbgInfoIConsumer


/////////////////////////////////////////////////////////////////////////////////////////////////////
/// DbgInfoCompoundConsumer
/// \brief Description: Constructor - need to pass in a low and high level consumer by reference, optional parameters are the userData and the pLocationResolver function pointer.
///                     If these are not supplied, a default implementation is provided which throws an assertion if used.
/// \param[in]          pHConsumer - High level consumer
/// \param[in]          pLConsumer - Low level consumer
/// \param[in]          pLocationResolver = &DbgInfoCompoundConsumer::DefaultLocationResolver - A function pointer to a LocationResolver function which given the following (HVarLocationType, LAddrType, LowLvlConsumer, void* userData) returns an LVarLocationType
/// \param[in]          pAddrResolver
/// \param[in]          pLineResolver
/// \param[in]          userData - needed for the location resolver - serves as meta data to be passed in by the user in order to resolve the location type
/// \param[in]          firstMappedLLAddr - should this consumer resolve HL addresses (= LL lines) to the first LL address or to all of them.
/////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename HAddrType, typename HLineType, typename HVarLocationType, typename LAddrType, typename LVarLocationType, typename LLineType>
DbgInfoCompoundConsumer<HAddrType, HLineType, HVarLocationType, LAddrType, LVarLocationType, LLineType>::DbgInfoCompoundConsumer(HighLvlConsumer* pHConsumer, LowLvlConsumer* pLConsumer, LocationResolver pLocationResolver, AddressResolver pAddrResolver, LineResolver pLineResolver, void* userData, bool firstMappedLLAddr)
    : m_pHConsumer(pHConsumer), m_pLConsumer(pLConsumer), m_pLocationResolver(pLocationResolver), m_pAddrResolver(pAddrResolver), m_pLineResolver(pLineResolver), m_pResolverUserData(userData), m_firstMappedLLAddr(firstMappedLLAddr)
{
    // We must assume these pointers to be non-nullptr:
    HWDBG_ASSERT(nullptr != m_pHConsumer);
    HWDBG_ASSERT(nullptr != m_pLConsumer);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
/// ~DbgInfoCompoundConsumer
/// \brief Description: Destructor
/////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename HAddrType, typename HLineType, typename HVarLocationType, typename LAddrType, typename LVarLocationType, typename LLineType>
DbgInfoCompoundConsumer<HAddrType, HLineType, HVarLocationType, LAddrType, LVarLocationType, LLineType>::~DbgInfoCompoundConsumer()
{
    delete m_pHConsumer;
    m_pHConsumer = nullptr;

    delete m_pLConsumer;
    m_pLConsumer = nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
/// GetLineFromAddress
/// \brief Description: receives a low level address and return a high level line - if there is no direct mapping it will get a line mapping to the nearest mapped address
/// \param[in]          lAddr - Low level address
/// \param[out]         o_hLine - High level line
/// \return             Success / failure.
/////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename HAddrType, typename HLineType, typename HVarLocationType, typename LAddrType, typename LVarLocationType, typename LLineType>
bool DbgInfoCompoundConsumer<HAddrType, HLineType, HVarLocationType, LAddrType, LVarLocationType, LLineType>::GetLineFromAddress(const LAddrType& lAddr, HLineType& o_hLine) const
{
    bool retVal = false;
    LLineType lLine;

    // Get LowLvlLine/HighLvlAddress from LowLvlAddress:
    if (m_pLConsumer->GetLineFromAddress(lAddr, lLine))
    {
        // Get HighLvlLine from LowLvlLine/HighLvlAddress:
        HAddrType hAddr = m_pLineResolver(lLine, m_pResolverUserData);
        retVal = m_pHConsumer->GetLineFromAddress(hAddr, o_hLine);
    }

    return retVal;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
/// GetAddressesFromLine
/// \brief Description: receives a high level line and return the list of low level addresses mapped to it or to the nearest low level line that has such a mapping
/// \param[in]          hLine - High level line
/// \param[out]         o_lAddresses - Vector of low level addresses
/// \param[in]          append - Whether to append to the output vector or clear it first
/// \return             Success / failure.
/////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename HAddrType, typename HLineType, typename HVarLocationType, typename LAddrType, typename LVarLocationType, typename LLineType>
bool DbgInfoCompoundConsumer<HAddrType, HLineType, HVarLocationType, LAddrType, LVarLocationType, LLineType>::GetAddressesFromLine(const HLineType& hLine, std::vector<LAddrType>& o_lAddresses, bool append, bool firstAddr) const
{
    bool retVal = false;
    std::vector<HAddrType> hAddresses;

    if (!append)
    {
        o_lAddresses.clear();
    }

    // Request high level addresses from the high level line
    if (m_pHConsumer->GetAddressesFromLine(hLine, hAddresses, false, firstAddr))
    {
        // Iterate over all the high level addresses:
        int numberOfHAddrs = (int)hAddresses.size();

        for (int i = 0; i < numberOfHAddrs; i++)
        {
            // Resolve to low level line:
            LLineType hAddrAsLLine = m_pAddrResolver(hAddresses[i], m_pResolverUserData);

            // We specify the last parameter to be true because we wish to append the results:
            // And fill the low level address vector:
            retVal = m_pLConsumer->GetAddressesFromLine(hAddrAsLLine, o_lAddresses, true, m_firstMappedLLAddr) || retVal;
        }
    }

    return retVal;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
/// GetNearestMappedLine
/// \brief Description: Gets nearest mapped line
/// \param[in]          hLine - High level line
/// \param[out]         o_nearestMappedHLine - High level line
/// \return             Success / failure.
/////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename HAddrType, typename HLineType, typename HVarLocationType, typename LAddrType, typename LVarLocationType, typename LLineType>
bool DbgInfoCompoundConsumer<HAddrType, HLineType, HVarLocationType, LAddrType, LVarLocationType, LLineType>::GetNearestMappedLine(const HLineType& hLine, HLineType& o_nearestMappedHLine) const
{
    bool retVal = false;

    // First, resolve the high level line:
    HLineType tryHLine;
    retVal = m_pHConsumer->GetNearestMappedLine(hLine, tryHLine);

    if (retVal)
    {
        // Check if this line is mapped to anything in the low level:
        std::vector<HAddrType> tryAddresses;
        retVal = m_pHConsumer->GetAddressesFromLine(tryHLine, tryAddresses, true);

        if (retVal)
        {
            retVal = false;
            int numberOfHAddrs = (int)tryAddresses.size();

            for (int i = 0; i < numberOfHAddrs; i++)
            {
                // Resolve each high level address to a low level line:
                LLineType tryLLine = m_pAddrResolver(tryAddresses[i], m_pResolverUserData);
                LLineType ignoredLine;

                if (m_pLConsumer->GetNearestMappedLine(tryLLine, ignoredLine))
                {
                    // If this low level line exists, we've succeeded:
                    o_nearestMappedHLine = tryHLine;
                    retVal = true;
                    break;
                }
            }
        }
    }

    return retVal;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
/// GetNearestMappedAddress
/// \brief Description: Gets nearest mapped address
/// \param[in]          lAddr - low level address
/// \param[out]         o_nearestMappedLAddress - low level address
/// \return             Success / failure.
/////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename HAddrType, typename HLineType, typename HVarLocationType, typename LAddrType, typename LVarLocationType, typename LLineType>
bool DbgInfoCompoundConsumer<HAddrType, HLineType, HVarLocationType, LAddrType, LVarLocationType, LLineType>::GetNearestMappedAddress(const LAddrType& lAddr, LAddrType& o_nearestMappedLAddress) const
{
    return m_pLConsumer->GetNearestMappedAddress(lAddr, o_nearestMappedLAddress);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
/// GetMappedAddresses
/// \brief Description: Gets all low level addresses currently mapped
/// \param[out]         o_lAddresses
/// \return             Success / failure.
/////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename HAddrType, typename HLineType, typename HVarLocationType, typename LAddrType, typename LVarLocationType, typename LLineType>
bool DbgInfoCompoundConsumer<HAddrType, HLineType, HVarLocationType, LAddrType, LVarLocationType, LLineType>::GetMappedAddresses(std::vector<LAddrType>& o_lAddresses) const
{
    bool retVal = false;
    o_lAddresses.clear();

    // Get the mapped addresses from
    std::vector<HAddrType> hAddrs;
    retVal = m_pHConsumer->GetMappedAddresses(hAddrs);

    if (retVal)
    {
        int numberOfHAddrs = (int)hAddrs.size();

        for (int i = 0; i < numberOfHAddrs; i++)
        {
            // Get the line number:
            LLineType lLineOrig = m_pAddrResolver(hAddrs[i], m_pResolverUserData);
            LLineType lLine;
            bool rcLn = m_pLConsumer->GetNearestMappedLine(lLineOrig, lLine);

            if (rcLn)
            {
                // Get its low level addresses:
                std::vector<LAddrType> currentLAddrs;
                retVal = m_pLConsumer->GetAddressesFromLine(lLine, o_lAddresses, true, m_firstMappedLLAddr) || retVal;
            }
        }
    }

    // We consider a success if we got any address:
    retVal = retVal && (0 < (int)o_lAddresses.size());

    return retVal;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
/// GetAddressVirtualCallStack
/// \brief Description: This function returns the virtual call stack where the first element is the (high level line, startLAddr) pair,
///                     from there we go up the code scopes, stopping on inline functions and taking their line types, and pairing those with the first address of the line
///                     All the (hLine,lAddr) pairs go into the o_stack vector and are returned
/// \param[in]          startLAddr - The low level address from which to start
/// \param[in,out]         io_stack - the vector of (hLine,LAddr) pairs to return
/// \return             Success / failure. Failure if we did not find any elements to return
/////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename HAddrType, typename HLineType, typename HVarLocationType, typename LAddrType, typename LVarLocationType, typename LLineType>
bool DbgInfoCompoundConsumer<HAddrType, HLineType, HVarLocationType, LAddrType, LVarLocationType, LLineType>::GetAddressVirtualCallStack(const LAddrType& startLAddr, std::vector<TwoLvlCallStackFrame>& io_stack) const
{
    bool retVal = false;

    // Get the low level call stack:
    std::vector<LowLvlCallStackFrame> llStack;
    bool rcLL = m_pLConsumer->GetAddressVirtualCallStack(startLAddr, llStack);

    if (rcLL)
    {
        // Iterate its frames:
        int numberOfLLFrames = (int)llStack.size();

        for (int i = 0; i < numberOfLLFrames; i++)
        {
            const LowLvlCallStackFrame& currentLLFrame = llStack[i];

            // Get the high level call stack for this frame:
            std::vector<HighLvlCallStackFrame> hlStack;
            HAddrType currentHStackStartAddr = m_pLineResolver(currentLLFrame.m_sourceLocation, m_pResolverUserData);
            bool rcHL = m_pHConsumer->GetAddressVirtualCallStack(currentHStackStartAddr, hlStack);

            if (rcHL)
            {
                // If we got at least one HL stack, we consider it a success:
                retVal = true;
                int numberOfHLFrames = (int)hlStack.size();

                for (int j = 0; j < numberOfHLFrames; j++)
                {
                    const HighLvlCallStackFrame& currentHLFrame = hlStack[j];
                    // Construct a two-level call stack:
                    TwoLvlCallStackFrame fullFrame;
                    fullFrame.m_sourceLocation = currentHLFrame.m_sourceLocation;

                    if (0 == j)
                    {
                        // Apply the low level values to the first frame only:
                        fullFrame.m_programCounter = currentLLFrame.m_programCounter;
                        fullFrame.m_functionBase = currentLLFrame.m_functionBase;
                        fullFrame.m_moduleBase = currentLLFrame.m_moduleBase;
                        fullFrame.m_functionName = currentHLFrame.m_functionName.empty() ? currentLLFrame.m_functionName : currentHLFrame.m_functionName;
                    }
                    else // 0 != j
                    {
                        // For other frames, try to resolve from the high level values:
                        LLineType llPCL = m_pAddrResolver(currentHLFrame.m_programCounter, m_pResolverUserData);
                        std::vector<LAddrType> llPCs;
                        bool rcPC = m_pLConsumer->GetAddressesFromLine(llPCL, llPCs, true, true);

                        if (rcPC && ((size_t)0 < llPCs.size()))
                        {
                            fullFrame.m_programCounter = llPCs[0];
                        }

                        LLineType llFBL = m_pAddrResolver(currentHLFrame.m_functionBase, m_pResolverUserData);
                        std::vector<LAddrType> llFBs;
                        bool rcFB = m_pLConsumer->GetAddressesFromLine(llFBL, llFBs, true, true);

                        if (rcFB && ((size_t)0 < llFBs.size()))
                        {
                            fullFrame.m_functionBase = llFBs[0];
                        }

                        LLineType llMBL = m_pAddrResolver(currentHLFrame.m_moduleBase, m_pResolverUserData);
                        std::vector<LAddrType> llMBs;
                        bool rcMB = m_pLConsumer->GetAddressesFromLine(llMBL, llMBs, true, true);

                        if (rcMB && ((size_t)0 < llMBs.size()))
                        {
                            fullFrame.m_moduleBase = llMBs[0];
                        }

                        fullFrame.m_functionName = currentHLFrame.m_functionName;
                    }

                    // Add the full frame to the output vector:
                    io_stack.push_back(fullFrame);
                }
            }
        }
    }

    return retVal;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
/// GetCachedAddresses
/// \brief Description: Gets the cached high level addresses, and for each of them gets all the the corresponding low level addresses
/// \param[in]          startLAddr - Low level address to start from
/// \param[in]          includeCurrentScope - whether to include the first scope or skip it.
/// \param[out]         o_cachedLAddresses - the cached addresses
/// \return             Success / failure.
/////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename HAddrType, typename HLineType, typename HVarLocationType, typename LAddrType, typename LVarLocationType, typename LLineType>
bool DbgInfoCompoundConsumer<HAddrType, HLineType, HVarLocationType, LAddrType, LVarLocationType, LLineType>::GetCachedAddresses(const LAddrType& startLAddr, bool includeCurrentScope, std::vector<LAddrType>& o_cachedLAddresses) const
{
    bool retVal = false;
    LLineType lLine;

    if (m_pLConsumer->GetLineFromAddress(startLAddr, lLine))
    {
        HAddrType hAddr = (HAddrType)lLine;
        std::vector<HAddrType> cachedHAddresses;

        if (m_pHConsumer->GetCachedAddresses(hAddr, includeCurrentScope, cachedHAddresses))
        {
            int numberOfHAddrs = (int)cachedHAddresses.size();

            for (int i = 0; i < numberOfHAddrs; i++)
            {
                LLineType currentLLine = m_pAddrResolver(cachedHAddresses[i], m_pResolverUserData);
                m_pLConsumer->GetAddressesFromLine(currentLLine, o_cachedLAddresses, true, m_firstMappedLLAddr);
            }
        }
    }

    retVal = (o_cachedLAddresses.size() > 0);
    return retVal;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
/// GetAddressStackDepth
/// \brief Description: Returns the high level stack depth.
/// \param[in]          lAddr
/// \return             the stack depth
/////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename HAddrType, typename HLineType, typename HVarLocationType, typename LAddrType, typename LVarLocationType, typename LLineType>
int DbgInfoCompoundConsumer<HAddrType, HLineType, HVarLocationType, LAddrType, LVarLocationType, LLineType>::GetAddressStackDepth(const LAddrType& lAddr) const
{
    int retVal = 0;
    LLineType lLine;

    if (m_pLConsumer->GetLineFromAddress(lAddr, lLine))
    {
        HAddrType hAddr = (HAddrType)lLine;
        retVal = m_pHConsumer->GetAddressStackDepth(hAddr);
    }

    return retVal;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
/// CopyHighToLowVariable
/// \brief Description: This function recursively fills a low level variable from a high level variable by:
/// 1. If the type is constant - copy the variable as is from the hVar
/// 2. Resolves the LowVarLocation from the HighVarLocation using the m_locationResolverHelper function
/// 3. Resolves the lower and upper address of the variable from HAddrType to LAddrType
/// 4. Copies all the other fields from the high level variable to the low level variable
/// 5. Recursively resolves the low level locations of all the high level member variables,
/// 6. creates matching low level member variables and adds them to the low level variable
/// \param[in]          lAddr - Low level address - needed for the resolver
/// \param[in]          hVar - High level variable to copy
/// \param[out]         o_lVar - the low level variable to return
/// \return             void
/////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename HAddrType, typename HLineType, typename HVarLocationType, typename LAddrType, typename LVarLocationType, typename LLineType>
void DbgInfoCompoundConsumer<HAddrType, HLineType, HVarLocationType, LAddrType, LVarLocationType, LLineType>::CopyHighToLowVariable(const LAddrType& lAddr, const HighLvlVariableInfo& hVar, LowLvlVariableInfo& o_lVar) const
{
    // Copy common parts:
    o_lVar.m_varName                        = hVar.m_varName;
    o_lVar.m_typeName                       = hVar.m_typeName;
    o_lVar.m_varSize                        = hVar.m_varSize;
    o_lVar.m_varEncoding                    = hVar.m_varEncoding;
    o_lVar.m_varIndirection                 = hVar.m_varIndirection;
    o_lVar.m_varIndirectionDetail           = hVar.m_varIndirectionDetail;
    o_lVar.m_isOutParam                     = hVar.m_isOutParam;

    // Resolve and copy Addresses:
    // Get Low address - Take Bottom high address, get its low addresses, and use the first one:
    LLineType lowVariablePC = m_pAddrResolver(hVar.m_lowVariablePC, m_pResolverUserData);
    std::vector<LAddrType> lAddresses;
    bool rcBottomAddr = m_pLConsumer->GetAddressesFromLine(lowVariablePC, lAddresses, false, true);

    if (rcBottomAddr && (0 < lAddresses.size()))
    {
        o_lVar.m_lowVariablePC = lAddresses[0];
    }

    // Get high address - Take top high level address, get its low level addresses, and use the last one:
    LLineType highVariablePC = m_pAddrResolver(hVar.m_highVariablePC, m_pResolverUserData);
    std::vector<LAddrType> hAddresses;
    bool rcTopAddr = m_pLConsumer->GetAddressesFromLine(highVariablePC, hAddresses, false, false);

    if (rcTopAddr && (0 < hAddresses.size()))
    {
        o_lVar.m_highVariablePC = hAddresses[hAddresses.size() - 1];
    }

    // If constant, copy the constant value:
    if (hVar.IsConst())
    {
        o_lVar.SetConstantValue(hVar.m_varSize, hVar.m_varValue.m_varConstantValue);
    }
    // Otherwise, resolve the Low level var location using the function pointer and copy it:
    else
    {
        HVarLocationType hVarLoc = hVar.m_varValue.m_varValueLocation;
        LVarLocationType lVarLoc;

        // Call to function pointer which returns low level var location:
        if (m_pLocationResolver(hVarLoc, lAddr, *m_pLConsumer, lVarLoc, m_pResolverUserData))
        {
            o_lVar.m_varValue.m_varValueLocation = lVarLoc;
        }
    }

    // Copy member variables:
    unsigned int numChildVars = (unsigned int)hVar.m_varMembers.size();

    for (unsigned int i = 0; i < numChildVars; ++i)
    {
        const HighLvlVariableInfo& hChildVar = hVar.m_varMembers[i];
        // Get a low level address from the high level Variable's min address:
        // HVarLocationType hChildVarLoc = hChildVar.m_varValue.m_varValueLocation;
        LowLvlVariableInfo o_lChildVar;
        // Recursive call:
        CopyHighToLowVariable(lAddr, hChildVar, o_lChildVar);
        o_lVar.m_varMembers.push_back(o_lChildVar);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
/// GetVariableInfoInCurrentScope
/// \brief Description: Returns the low level variable given a high level var name and an low level address.
///                     Variable is mostly the same except for the high/low addresses and the var location
/// \param[in]          startLAddr - Low level address
/// \param[in]          variableName - High level variable name
/// \param[out]         o_variable - low level variable to return
/// \return             Success / failure.
/////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename HAddrType, typename HLineType, typename HVarLocationType, typename LAddrType, typename LVarLocationType, typename LLineType>
bool DbgInfoCompoundConsumer<HAddrType, HLineType, HVarLocationType, LAddrType, LVarLocationType, LLineType>::GetMatchingVariableInfoInCurrentScope(const LAddrType& startLAddr, typename TwoLvlIConsumer::VarMatchFunc pfnMatch, const void* pMatchData, LowLvlVariableInfo& o_variable) const
{
    bool retVal = false;
    LLineType lLine;

    // Get Low level line:
    if (m_pLConsumer->GetLineFromAddress(startLAddr, lLine))
    {
        // Translate to High level address:
        HAddrType hAddr = (HAddrType)lLine;
        HighLvlVariableInfo hVar;

        // Get high level variable:
        if (m_pHConsumer->GetMatchingVariableInfoInCurrentScope(hAddr, pfnMatch, pMatchData, hVar))
        {
            // Copy high level variable to low level variable, resolving necessary fields:
            CopyHighToLowVariable(startLAddr, hVar, o_variable);
            retVal = true;
        }
    }

    return retVal;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
/// GetFrameBase
/// \brief Description: Gets the frame base which is a low level variable location, this is done by
///                     getting the high level frame base and then calling the function pointer to resolve it
/// \param[in]          startLAddr - Low level address
/// \param[in]          varName - High level variable name
/// \param[out]         o_lFrameBase - frame base to return
/// \return             Success / failure.
/////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename HAddrType, typename HLineType, typename HVarLocationType, typename LAddrType, typename LVarLocationType, typename LLineType>
bool DbgInfoCompoundConsumer<HAddrType, HLineType, HVarLocationType, LAddrType, LVarLocationType, LLineType>::GetFrameBase(LAddrType startLAddr, const std::string& varName, LVarLocationType& o_lFrameBase) const
{
    bool retVal = false;
    LLineType lLine;

    if (m_pLConsumer->GetLineFromAddress(startLAddr, lLine))
    {
        HAddrType hAddr = (HAddrType)lLine;
        HVarLocationType hFrameBase;
        bool rcFB = m_pHConsumer->GetFrameBase(hAddr, varName, hFrameBase);

        if (rcFB)
        {
            retVal = m_pLocationResolver(hFrameBase, startLAddr, *m_pLConsumer, o_lFrameBase, nullptr);
        }
    }

    return retVal;
}

/// -----------------------------------------------------------------------------------------------
/// ListVariablesFromAddress
/// \brief Description: Translates scope and gets a list of all high level variables defined in address's scope and all containing scopes
/// \param[in]          addr - the address in which to look for the scope
/// \param[in]          stackFrameDepth - the stack frame depth in which to look
/// \param[in]          finalMembers - true = get lowest level variables possible (no children). false = get top-level variables
/// \param[out]         o_variableNames - the list of variable names in the address's scope
/// \return True : At least one variable found
/// \return False: Otherwise
/// -----------------------------------------------------------------------------------------------
template<typename HAddrType, typename HLineType, typename HVarLocationType, typename LAddrType, typename LVarLocationType, typename LLineType>
bool DbgInfoCompoundConsumer<HAddrType, HLineType, HVarLocationType, LAddrType, LVarLocationType, LLineType>::ListVariablesFromAddress(const LAddrType& addr, int stackFrameDepth, bool finalMembers, std::vector<std::string>& o_variableNames) const
{
    bool retVal = false;

    if (m_pLConsumer && m_pHConsumer)
    {
        LLineType lLine;

        if (m_pLConsumer->GetLineFromAddress(addr, lLine))
        {
            HAddrType hAddr = (HAddrType)lLine;
            retVal = m_pHConsumer->ListVariablesFromAddress(hAddr, stackFrameDepth, finalMembers, o_variableNames);
        }
    }

    return retVal;
}



} /// namespace HwDbg

#endif ///DBGINFOCOMPOUNDCONSUMER_H_
