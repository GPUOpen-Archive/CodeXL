//==============================================================================
// Copyright (c) 2012-2015 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools
/// \file
/// \brief  Definition of the Debug Information lines
//==============================================================================
#ifndef DBGINFOLINES_H_
#define DBGINFOLINES_H_

///////////////////////////////////////////////////////////////////////////////////////
/// Implementation              /// Lines               /// Addresses               ///
///////////////////////////////////////////////////////////////////////////////////////
/// OCL Compiler                /// OpenCL + H file(s)  /// AMDIL lines in temp file///
/// HSA C++ AMP compiler        /// Source file(s)      /// Source file             ///
/// MS C++ AMP compiler         /// Source file(s)      /// DX ASM                  ///
/// DXX                         /// DX ASM              /// BRIG                    ///
/// Finalizer                   /// BRIG                /// ISA                     ///
///////////////////////////////////////////////////////////////////////////////////////

/// Local:
#include <DbgInfoDefinitions.h>

/// STL:
#include <map>
#include <string>
#include <vector>

/// How many addresses to read ahead when we are searching for an address mapped to a specific line
#define DIL_DEFAULT_ADDR_READAHEAD 20
/// How many addresses to read back when we are searching for an address mapped to a specific line
#define DIL_DEFAULT_ADDR_READBACK 0
/// How many lines to read ahead when we are searching for a line mapped to a specific address:
#define DIL_DEFAULT_LINE_READAHEAD 10
/// How many lines to read back when we are searching for a line mapped to a specific address:
#define DIL_DEFAULT_LINE_READBACK 3

namespace HwDbg
{
///////////////////////////////////////////////////////////////////////////////////////////////////
/// \class LineNumberMapping
/// \brief Provides a mapping of "line numbers" to "addresses" as specified in debug information.
/// \brief Invariants:  The definition of "line numbers" and "addresses" may change, but
/// the following will always apply:
/// 1. Any address may only be mapped to a single line number.
/// 2. A line number may be mapped to more than one address. The
///    addresses belonging to the same line do not have to be
///    contiguous (e.g inlined functions, for statements, code motion)
/// 3. Each type must be contextually unique, i.e. no duplications.
///    For example, if the line number information is actual code
///    lines, the data structure must also contain the file name or
///    any other way to separate line X in file Y from line X in file Z
/// 4. The same data structure must be applicable to any line / address
///    definition combination
/// 5. When a Line is not mapped to any address, it is not considered
///    debuggable code (i.e. it is a comment)
/// 6. When an address is not mapped to any line, it is considered to
///    be mapped to the same line as it nearest predecessor which is
///    mapped to a line (this implies that a line may break down to
///    many addresses).
/// \brief Requirements: Addr and Line must have comparison operators < and ==
/// and the assignment operator =.
/// Both Line and Addr must have the following notions / operators:
/// A "zero value", which does not have to be unique, signifying the
/// beginning of the address space.
/// op.-- go to the predecessor of this address, or keep it the same
///    if it is the zero value.
/// op.++ go to the Successor of this address.
/// operator bool which returns false iff the value is a zero value
/// \brief Note: A zero value does not have to be a minimum, it is allowed to
/// have values smaller than it - being a zero value just means that
/// this value does not have an immediate predecessor.
///
///////////////////////////////////////////////////////////////////////////////////////
/// Implementation              /// Lines               /// Addresses               ///
///////////////////////////////////////////////////////////////////////////////////////
/// OCL Compiler                /// OpenCL + H file(s)  /// AMDIL lines in temp file///
/// HSA C++ AMP compiler        /// Source file(s)      /// Source file             ///
/// MS C++ AMP compiler         /// Source file(s)      /// DX ASM                  ///
/// DXX                         /// DX ASM              /// BRIG                    ///
/// Finalizer                   /// BRIG                /// ISA                     ///
///////////////////////////////////////////////////////////////////////////////////////
/// \tparam typename AddrType: The Address type
/// \tparam  typename LineType: The Line Type
///////////////////////////////////////////////////////////////////////////////////////////////////

typedef unsigned long long HwDbgUInt64;
template <typename AddrType, typename LineType> class LineNumberMapping
{
public:
    /// Constructor: Set the number of lines/addresses to read back/forward when searching
    LineNumberMapping(int addrReadAhead = DIL_DEFAULT_ADDR_READAHEAD, int addrReadBack = DIL_DEFAULT_ADDR_READBACK, int lineReadAhead = DIL_DEFAULT_LINE_READAHEAD, int lineReadBack = DIL_DEFAULT_LINE_READBACK)
        : m_addrReadAhead(addrReadAhead), m_addrReadBack(addrReadBack), m_lineReadAhead(lineReadAhead), m_lineReadBack(lineReadBack) {}
    ~LineNumberMapping() { ClearMap(); };
    /// Given a line and an address, add them to the internal mappings
    bool AddLineMapping(const LineType& line, const AddrType& addr);
    /// Clear internal mappings
    void ClearMap();
    /// Gets the line mapped to the specified address and returns it as an out param, return false if not found
    bool GetLineFromAddress(const AddrType& addr, LineType& o_line) const;
    /// Gets the addresses mapped to the specified line and returns them as an out param, return false if not found, append specifies whether to append to the existing addresses or clear the vector
    bool GetAddressesFromLine(const LineType& line, std::vector<AddrType>& o_addrs, bool append = false) const;
    /// Returns a list of all the mapped lines
    bool GetMappedLines(std::vector<LineType>& o_lines) const;
    /// Returns a list of all the mapped addresses
    bool GetMappedAddresses(std::vector<AddrType>& o_addrs) const;
    /// Returns a list of all the mapped first addresses
    bool GetMappedFirstAddresses(std::vector<AddrType>& o_addrs) const;
    /// Get the nearest mapped line and returns it - the distance of the search is determined by the class' members, return false if a mapped line is not found
    bool GetNearestMappedLine(const LineType& line, LineType& o_mappedLine) const;
    /// Get the nearest mapped address and returns it - the distance of the search is determined by the class' members, return false if a mapped address is not found
    bool GetNearestMappedAddress(const AddrType& addr, AddrType& o_mappedAddr) const;

private:
    /// 1:1 mapping of address to line
    typedef std::map<AddrType, LineType> atlMap;
    /// 1:N mapping of line to addresses
    typedef std::map<LineType, std::vector<AddrType>* > ltaMap;
    /// Disallow use of assignment operator
    LineNumberMapping& operator=(const LineNumberMapping& other);
    /// Disallow use of copy constructor:
    LineNumberMapping(const LineNumberMapping& other);

    atlMap m_addrToLine;                ///< 1:1 Mapping of AddrType toLineType
    ltaMap m_lineToAddr;                ///< 1:N Mapping of LineType to AddrType
    std::vector<AddrType> m_mappedAddrs;        ///< Vector of all mapped addresses
    std::vector<AddrType> m_mappedFirstAddrs;   ///< Vector of all mapped first addresses
    std::vector<LineType> m_mappedLines;        ///< Vector of all mapped lines
    const int m_addrReadAhead;          ///< Number of addresses to look ahead - Default is DIL_DEFAULT_ADDR_READAHEAD
    const int m_addrReadBack;           ///< Number of addresses to look back - Default is DIL_DEFAULT_ADDR_READBACK
    const int m_lineReadAhead;          ///< Number of lines to look ahead - Default is DIL_DEFAULT_LINE_READAHEAD
    const int m_lineReadBack;           ///< Number of lines to look back - Default is DIL_DEFAULT_LINE_READBACK
};
/// -----------------------------------------------------------------------------------------------
/// \class FileLocation
/// \brief Description: Helper class which can be used as a LineType or AddrType, This can be used as a Line OR an Addr
/// -----------------------------------------------------------------------------------------------
struct DBGINF_API FileLocation
{
public:
    /// Constructor Receives a file path and a line number
    FileLocation(const std::string& fullPath = "", HwDbgUInt64 lineNum = 0);
    FileLocation(const FileLocation& other);
    /// Destructor
    ~FileLocation();
    /// Move semantics
#ifdef HWDBGINFO_MOVE_SEMANTICS
    FileLocation(FileLocation&& other);
    FileLocation& operator= (FileLocation&& other);
#endif
    /// Helper function to dump to string
    static void AsString(const FileLocation& fileLoc, std::string& o_outputString);
    //@{
    /// Operator overloading
    bool operator< (const FileLocation& other) const;
    bool operator== (const FileLocation& other) const;
    FileLocation& operator= (const FileLocation& other);
    FileLocation& operator++();
    FileLocation operator++ (int);
    FileLocation& operator--();
    FileLocation operator-- (int);
    operator bool () const;
    operator HwDbgUInt64() const;
    FileLocation& operator= (const HwDbgUInt64& other);
    //@}
    const char* fullPath() const { return m_fullPath;}; /// Accessor
    FileLocation& setFullPath(const char* pPath, size_t n = 0);
    FileLocation& setFullPath(const std::string& strPath) {return setFullPath(strPath.c_str(), strPath.length()); };
    FileLocation& clearFullPath();

    char* m_fullPath; ///< The full path of the file
    HwDbgUInt64 m_lineNum; ///< The line number
};

/// -----------------------------------------------------------------------------------------------
/// AddLineMapping
/// \brief Description: Add a line to address mapping
/// \param[in]          line - The line
/// \param[in]          addr - The address
/// \return True if :   We are trying to add a new address
/// \return False if:   We have already added this address
/// -----------------------------------------------------------------------------------------------
template<typename AddrType, typename LineType>
bool LineNumberMapping<AddrType, LineType>::AddLineMapping(const LineType& line, const AddrType& addr)
{
    bool retVal = false;
    typename atlMap::iterator firstMapFindIter = m_addrToLine.find(addr);
    typename atlMap::iterator firstMapEndIter = m_addrToLine.end();
    bool isNewAddr = (firstMapEndIter == firstMapFindIter);

    // An address may only be mapped once!:
    if (isNewAddr)
    {
        retVal = true;
        // Add address to the vector of mapped addresses:
        m_mappedAddrs.push_back(addr);

        // Add address to line the mapping:
        m_addrToLine[addr] = line;

        typename ltaMap::iterator secondMapFindIter = m_lineToAddr.find(line);
        typename ltaMap::iterator secondMapEndIter = m_lineToAddr.end();
        bool isNewLine = (secondMapEndIter == secondMapFindIter);

        //If we have a new line, we need to add it to the lines vector and initialize a new address vector for this specific line:
        if (isNewLine)
        {
            // Add Line to vector:
            m_mappedLines.push_back(line);

            // This is a new "first" address:
            m_mappedFirstAddrs.push_back(addr);

            // Create & initialize new addresses vector for the new line:
            std::vector<AddrType>* addressesMappedToLine = new (std::nothrow)std::vector<AddrType>;
            if (nullptr != addressesMappedToLine)
            {
                addressesMappedToLine->push_back(addr);
                m_lineToAddr[line] = addressesMappedToLine;
            }
        }
        else // If this is not a new line, add the address to the existing line's address vector
        {
            secondMapFindIter->second->push_back(addr);
        }
    }
    else // !isNewAddr
    {
        // If the address is already mapped, return success if it's simply a duplicate mapping to the same line number.
        // Since we enforce a one-to-many relation of line to addresses, any other value means information is discarded,
        // so we will consider it a failure:
        if (firstMapFindIter->second == line)
        {
            retVal = true;
        }
    }

    return retVal;
};

/// -----------------------------------------------------------------------------------------------
/// ClearMap
/// \brief Description: Clears the mappings
/// -----------------------------------------------------------------------------------------------
template<typename AddrType, typename LineType>
void LineNumberMapping<AddrType, LineType>::ClearMap()
{
    m_addrToLine.clear();
    // m_lineToAddr.clear(); // TO_DO: Find out why removing this memory leak causes a crash in some situations
    m_mappedAddrs.clear();
    m_mappedFirstAddrs.clear();
    m_mappedLines.clear();
};

/// -----------------------------------------------------------------------------------------------
/// GetLineFromAddress
/// \brief Description: Gets the line mapped to the current address
/// \param[in]          addr - The address for which to retrieve the line
/// \param[out]         o_line - The output line to return
/// \return True : if a mapping is found from address to line
/// \return False: Otherwise
/// -----------------------------------------------------------------------------------------------
template<typename AddrType, typename LineType>
bool LineNumberMapping<AddrType, LineType>::GetLineFromAddress(const AddrType& addr, LineType& o_line) const
{
    bool retVal = false;

    typename atlMap::const_iterator findIter = m_addrToLine.find(addr);
    typename atlMap::const_iterator endIter = m_addrToLine.end();

    if (endIter != findIter)
    {
        retVal = true;
        o_line = findIter->second;
    }

    return retVal;
};

/// -----------------------------------------------------------------------------------------------
/// GetAddressesFromLine
/// \brief Description: Gets the addresses mapped to the current line
/// \param[in]          line - the line for which to retrieve the addresses
/// \param[out]         o_addrs - the vector of addresses mapped to the line
/// \param[in]          append - Whether to append to the received vector (Default: true)
/// \return True : If at least one address was found
/// \return False: Otherwise
/// -----------------------------------------------------------------------------------------------
template<typename AddrType, typename LineType>
bool LineNumberMapping<AddrType, LineType>::GetAddressesFromLine(const LineType& line, std::vector<AddrType>& o_addrs, bool append) const
{
    if (!append)
    {
        o_addrs.clear();
    }

    typename ltaMap::const_iterator findIter = m_lineToAddr.find(line);
    typename ltaMap::const_iterator endIter = m_lineToAddr.end();

    // If this line has valid addresses:
    if (endIter != findIter)
    {
        // Add the addresses to the output parameter:
        const std::vector<AddrType>& addrs = *(findIter->second);
        int numOfAddrs = (int)addrs.size();

        for (int i = 0; i < numOfAddrs; i++)
        {
            o_addrs.push_back(addrs[i]);
        }
    }

    // Return true if we have at least one mapped address:
    return (0 != o_addrs.size());
};

/// -----------------------------------------------------------------------------------------------
/// GetMappedLines
/// \brief Description: Return all the lines which are mapped to addresses
/// \param[out]         o_lines - mapped lines
/// \return True : If at least one line was found
/// \return False: Otherwise
/// -----------------------------------------------------------------------------------------------
template<typename AddrType, typename LineType>
bool LineNumberMapping<AddrType, LineType>::GetMappedLines(std::vector<LineType>& o_lines) const
{
    o_lines.clear();

    // Get lines from mapped lines vector
    int numberOfMappedLines = (int)m_mappedLines.size();

    for (int i = 0; i < numberOfMappedLines; i++)
    {
        o_lines.push_back(m_mappedLines[i]);
    }

    return (0 != o_lines.size());
};

/// -----------------------------------------------------------------------------------------------
/// GetMappedAddresses
/// \brief Description: Return all the addresses which are mapped to lines
/// \param[out]         o_addrs - Mapped addresses
/// \return True : If at least one address was found
/// \return False: Otherwise
/// -----------------------------------------------------------------------------------------------
template<typename AddrType, typename LineType>
bool LineNumberMapping<AddrType, LineType>::GetMappedAddresses(std::vector<AddrType>& o_addrs) const
{
    o_addrs.clear();

    // Get addresses from mapped addresses vector
    int numberOfMappedAddrs = (int)m_mappedAddrs.size();

    for (int i = 0; i < numberOfMappedAddrs; i++)
    {
        o_addrs.push_back(m_mappedAddrs[i]);
    }

    return (0 != o_addrs.size());
};

/// -----------------------------------------------------------------------------------------------
/// GetMappedFirstAddresses
/// \brief Description: Return all the addresses which are the first ones mapped to lines
/// \param[out]         o_addrs - Mapped addresses
/// \return True : If at least one address was found
/// \return False: Otherwise
/// -----------------------------------------------------------------------------------------------
template<typename AddrType, typename LineType>
bool LineNumberMapping<AddrType, LineType>::GetMappedFirstAddresses(std::vector<AddrType>& o_addrs) const
{
    o_addrs.clear();

    // Get addresses from mapped addresses vector
    int numberOfMappedAddrs = (int)m_mappedFirstAddrs.size();

    for (int i = 0; i < numberOfMappedAddrs; i++)
    {
        o_addrs.push_back(m_mappedFirstAddrs[i]);
    }

    return (0 != o_addrs.size());
};

/// -----------------------------------------------------------------------------------------------
/// GetNearestMappedLine
/// \brief Description: Get the nearest mapped line to the specified line, search range determined by
/// the class members \a m_lineReadAhead and \a m_lineReadBack
/// \param[in]          line - specified line
/// \param[out]         o_mappedLine - nearest mapped line
/// \return True : If a mapped line was found withing the specified range
/// \return False: Otherwise
/// -----------------------------------------------------------------------------------------------
template<typename AddrType, typename LineType>
bool LineNumberMapping<AddrType, LineType>::GetNearestMappedLine(const LineType& line, LineType& o_mappedLine) const
{
    bool retVal = false;

    LineType foundLine = line;
    typename ltaMap::const_iterator endIter = m_lineToAddr.end();

    // First, try to read ahead - including current line:
    for (int i = 0; i < m_lineReadAhead; i++)
    {
        // Check if the current line has a mapping:
        typename ltaMap::const_iterator findIter = m_lineToAddr.find(foundLine);

        if (endIter != findIter)
        {
            retVal = true;
            o_mappedLine = foundLine;
            break;
        }

        ++foundLine;
    }

    // After m_lineReadAhead steps, if we have not found a mapped line, start looking back behind the line:
    if (!retVal)
    {
        // Do not include the original line:
        foundLine = line;
        -- foundLine;

        for (int i = 1; i < m_lineReadBack; i++)
        {
            // Check if the current line has a mapping:
            typename ltaMap::const_iterator findIter = m_lineToAddr.find(foundLine);

            if (endIter != findIter)
            {
                retVal = true;
                o_mappedLine = foundLine;
                break;
            }

            // If we have reached the beginning - Line is supposed to be false at the beginning:
            if (!foundLine)
            {
                break;
            }

            // decrement line:
            --foundLine;
        }
    }

    return retVal;
};

/// -----------------------------------------------------------------------------------------------
/// GetNearestMappedAddress
/// \brief Description: Get the nearest mapped address and put it into o_mappedAddr.
/// Search range determined by the class members \a m_addrReadAhead and \a m_addrReadBack
/// \param[in]          addr - address to look for
/// \param[out]         o_mappedAddr - nearest mapped address
/// \return True : If a mapped address  was found withing the specified range
/// \return False: Otherwise
/// -----------------------------------------------------------------------------------------------
template<typename AddrType, typename LineType>
bool LineNumberMapping<AddrType, LineType>::GetNearestMappedAddress(const AddrType& addr, AddrType& o_mappedAddr) const
{
    bool retVal = false;

    AddrType foundAddr = addr;
    typename atlMap::const_iterator endIter = m_addrToLine.end();

    // Look ahead for the address:
    for (int i = 0; i < m_addrReadAhead; i++)
    {
        typename atlMap::const_iterator findIter = m_addrToLine.find(foundAddr);

        // If we have found the address:
        if (endIter != findIter)
        {
            // Return true and output the address:
            retVal = true;
            o_mappedAddr = foundAddr;
            break;
        }

        ++foundAddr;
    }

    // If the address is not ahead, look behind:
    if (!retVal)
    {
        // Do not include the original address:
        foundAddr = addr;
        --foundAddr;

        for (int i = 1; i < m_addrReadBack; i++)
        {
            typename atlMap::const_iterator findIter = m_addrToLine.find(foundAddr);

            // If we have found the address:
            if (endIter != findIter)
            {
                // Return true and output the address:
                retVal = true;
                o_mappedAddr = foundAddr;
                break;
            }

            // If we have reached the beginning break- Address is supposed to be false at the beginning:
            if (!foundAddr)
            {
                break;
            }

            // decrement Address:
            --foundAddr;
        }
    }

    return retVal;
};

}
#endif // DBGINFOLINES_H_
