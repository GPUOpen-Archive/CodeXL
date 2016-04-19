//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief StackTrace Atp File writer and parser
//==============================================================================

#ifndef _STACK_TRACE_ATP_FILE_H
#define _STACK_TRACE_ATP_FILE_H

#include <AMDTOSWrappers/Include/osOSDefinitions.h>

#include "StackTracer.h"
#include "../sprofile/AtpFile.h"

/// Object representing an entry in the symbol file (or symbol file part of the atp file)
struct SymbolFileEntry
{
    std::string m_strModName;   ///< the module name (i.e. "ocl" or "hsa")
    osThreadId  m_tid;          ///< the thread ID
    std::string m_strAPIName;   ///< the name of the API whose symbol information is given
    StackEntry* m_pStackEntry;  ///< the stack entry object for this entry

    /// Constructor
    /// \param strModName the module name of the API
    /// \param strAPIName the name of the API
    /// \param pStackEntry the stack entry object
    SymbolFileEntry(const std::string& strModName, const std::string& strAPIName, StackEntry* pStackEntry) : m_strModName(strModName), m_strAPIName(strAPIName), m_pStackEntry(pStackEntry) {}

    /// Destructor
    virtual ~SymbolFileEntry() {}

private:
    /// Hide copy constructor
    SymbolFileEntry(const SymbolFileEntry& entry);

    /// Hide default assignment operator
    SymbolFileEntry& operator=(const SymbolFileEntry& entry);
};

/// map from thread id to list of symbol file entries
typedef std::map<osThreadId, std::vector<SymbolFileEntry*> > SymbolEntryMap;

//------------------------------------------------------------------------------------
/// stack trace result
//------------------------------------------------------------------------------------
class StackTraceAtpFilePart : public IAtpFilePart, public IAtpFilePartParser, public BaseParser<SymbolFileEntry>
{
public:
    /// Constructor
    /// \param strModName Stack trace module name
    /// \param config Config object
    StackTraceAtpFilePart(const std::string& strModName, const Config& config, bool shouldReleaseMemory = true);

    /// Destructor
    ~StackTraceAtpFilePart();

    /// Write header section
    /// If a AptFilePart wants to output to header section, implement this method
    /// \param sout Output stream
    void WriteHeaderSection(SP_fileStream& sout);

    /// Write content section
    /// \param sout Output stream
    /// \param strTmpFilePath Output fragment files path
    /// \param strPID child process ID
    /// \return true if any contents were written, false otherwise
    bool WriteContentSection(SP_fileStream& sout, const std::string& strTmpFilePath, const std::string& strPID);

    /// Save atp file part into a separate file (Called only in compatibility mode)
    /// \param strTmpFilePath Output fragment files path
    /// \param strPID child process ID
    void SaveToFile(const std::string& strTmpFilePath, const std::string& strPID);

    // For IAtpFilePartParser

    /// Parse input stream
    /// \param in Input stream
    /// \return True if succeeded
    bool Parse(std::istream& in, std::string& outErrorMsg) override;

    /// Parse header
    /// \param strKey Key name
    /// \param strVal Value
    /// \return True if succeeded
    bool ParseHeader(const std::string& strKey, const std::string& strVal);

private:
    /// Parse symbol entry from .atp or .st file
    /// \param[in]    buf                symbol string
    /// \param[out]   pSymbolFileEntry   newly created SymbolFileEntry object
    /// \return True if succeed.
    bool ParseSymbolEntry(const std::string& buf, SymbolFileEntry*& pSymbolFileEntry);

    SymbolEntryMap m_SymbolEntryMap; ///< StackEntry Map key = threadID
};

#endif // _STACK_TRACE_ATP_FILE_H
