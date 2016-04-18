//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file pdGDBDataStructs.h
///
//==================================================================================

//------------------------------ pdGDBDataStructs.h ------------------------------

#ifndef __PDGDBDATASTRUCTS_H
#define __PDGDBDATASTRUCTS_H

// POSIX:
#include <sys/types.h>

// Infra
#include <AMDTBaseTools/Include/gtList.h>
#include <AMDTBaseTools/Include/gtASCIIString.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osCallStack.h>
#include <AMDTOSWrappers/Include/osFilePath.h>


// ----------------------------------------------------------------------------------
// Struct Name:          pdGDBData
// General Description: Base class for all GDB data structures.
// Author:               Yaki Tebeka
// Creation Date:        7/1/2007
// ----------------------------------------------------------------------------------
struct pdGDBData
{
    // A list of available sub-classes:
    enum pdGDBDataType
    {
        PD_GDB_THREAD_DATA,
        PD_GDB_THREAD_DATA_LIST,
        PD_GDB_CALL_STACK_DATA,
        PD_GDB_PROCESS_ID,
        PD_GDB_SOURCE_DATA,
        PD_GDB_LIBRARY_DATA,
        PD_GDB_STRING_DATA,
        PD_LOCALS_VARIABLES_LIST_DATA,
        PD_LOCAL_VARIABLE_VALUE_DATA,
        PD_GDB_BREAKPOINT_INDEX_DATA,
        PD_GDB_VARIABLE_TYPE_DATA,
        PD_GDB_HOST_STEP_RESULT_DATA
    };

public:
    virtual ~pdGDBData();

    // Must be implemented by sub-classes:
    virtual pdGDBDataType type() const = 0;
};


// ----------------------------------------------------------------------------------
// Struct Name:          pdGDBThreadData
// General Description: Contains a debugged process thread data.
// Author:               Yaki Tebeka
// Creation Date:        7/1/2007
// ----------------------------------------------------------------------------------
struct pdGDBThreadData : public pdGDBData
{
public:
    pdGDBThreadData();
    virtual ~pdGDBThreadData();

    // Overrides pdGDBData:
    virtual pdGDBDataType type() const;

public:
    // The OS thread id:
    osThreadId _OSThreadId;

    // GDBs internal thread id:
    int _gdbThreadId;

    // The threads instruction pointer pointed address:
    osInstructionPointer _threadIPLocation;

    // Contains true iff this thread is GDB's active thread.
    bool _isGDBsActiveThread;

    // Contains true iff this is a driver thread:
    bool _isDriverThread;
};


// ----------------------------------------------------------------------------------
// Struct Name:          pdGDBThreadDataList : public pdGDBData
// General Description: Contains a list of debugged process threads data.
// Author:               Yaki Tebeka
// Creation Date:        7/1/2007
// ----------------------------------------------------------------------------------
struct pdGDBThreadDataList : public pdGDBData
{
public:
    pdGDBThreadDataList();
    virtual ~pdGDBThreadDataList();

    // Overrides pdGDBData:
    virtual pdGDBDataType type() const;

public:
    // A list of debugged process thread's data:
    gtList<pdGDBThreadData> _threadsDataList;
};


// ----------------------------------------------------------------------------------
// Struct Name:          pdGDBThreadDataList : public pdGDBData
// General Description: Contains a list of debugged process threads data.
// Author:               Yaki Tebeka
// Creation Date:        14/3/2007
// ----------------------------------------------------------------------------------
struct pdGDBCallStack : public pdGDBData
{
public:
    pdGDBCallStack();
    virtual ~pdGDBCallStack();

    // Overrides pdGDBData:
    virtual pdGDBDataType type() const;

public:
    // The current thread's call stack:
    osCallStack _callStack;
};


// ----------------------------------------------------------------------------------
// Struct Name:          pdGDBProcessId : public pdGDBData
// General Description: Contains a process id (pid).
// Author:               Yaki Tebeka
// Creation Date:        29/8/2007
// ----------------------------------------------------------------------------------
struct pdGDBProcessId : public pdGDBData
{
public:
    pdGDBProcessId();
    virtual ~pdGDBProcessId();

    // Overrides pdGDBData:
    virtual pdGDBDataType type() const;

public:
    // The process pid:
    pid_t _processPid;
};

// ----------------------------------------------------------------------------------
// Struct Name:          pdGDBSourceCodeData : public pdGDBData
// General Description: Contains a source code location, made up from a source code
//                      file path and a line number
// Author:               Uri Shomroni
// Creation Date:        17/2/2009
// ----------------------------------------------------------------------------------
struct pdGDBSourceCodeData : public pdGDBData
{
public:
    pdGDBSourceCodeData(const osFilePath& sourceCodeFilePath, unsigned int lineNumber);
    virtual ~pdGDBSourceCodeData();

    // Overrides pdGDBData:
    virtual pdGDBDataType type() const;

private:
    // Do not allow the use of my default constructor:
    pdGDBSourceCodeData();

public:
    // The source code file path:
    osFilePath _sourceCodeFilePath;

    // The source code line number:
    unsigned int _lineNumber;
};

// ----------------------------------------------------------------------------------
// Class Name:           t pdGDBLibraryData : public pdGDBData
// General Description: Holds the output of a "info sharedlibrary 0xfeedface" command
//                      This output is the path to the library containing the instruction
// Author:               Uri Shomroni
// Creation Date:        22/2/2009
// ----------------------------------------------------------------------------------
struct pdGDBLibraryData : public pdGDBData
{
public:
    pdGDBLibraryData();
    virtual ~pdGDBLibraryData();

    // Overrides pdGDBData:
    virtual pdGDBDataType type() const;

public:
    // The file path to the library:
    osFilePath _libraryFilePath;
};

// ----------------------------------------------------------------------------------
// Struct Name:          pdGDBStringData : public pdGDBData
// General Description: Contains a string output from gdb.
// Author:               Yaki Tebeka
// Creation Date:        29/8/2007
// ----------------------------------------------------------------------------------
struct pdGDBStringData : public pdGDBData
{
public:
    pdGDBStringData(const gtASCIIString& gdbOutputString);
    virtual ~pdGDBStringData();

    // Overrides pdGDBData:
    virtual pdGDBDataType type() const;

private:
    // Do not allow the use of my default constructor:
    pdGDBStringData();

public:
    // The gdb output string:
    gtASCIIString _gdbOutputString;
};

//////////////////////////////////////////////////////////////////////
/// \struct pdGDBFrameLocalsData
/// \brief store of frame local variables and values
///
struct pdGDBFrameLocalsData : public pdGDBData
{
public:
    ///////////////////////////////////////////////
    /// \brief Constructor
    ///
    /// \param[in]  locals a list of "local variable name" <-> "local variable value" pairs
    pdGDBFrameLocalsData(const gtList < std::pair<gtString, gtString> >& locals);

    ///////////////////////////////////////////////
    /// \brief Get current instance type
    ///
    /// \return data type of the pdGDBFrameLocalVariableValue structure
    virtual pdGDBDataType type() const override;

    ///////////////////////////////////////////////
    /// \brief Standard virtual destructor
    ///
    virtual ~pdGDBFrameLocalsData();

    gtList < std::pair<gtString, gtString> >    _localsVariables; ///< list of "local variable name" <-> "local variable value" pairs


private:
    ///////////////////////////////////////////////
    /// \brief Standard constructor (unavailable)
    ///
    pdGDBFrameLocalsData();
};

//////////////////////////////////////////////////////////////////////
/// \struct pdGDBFrameLocalVariableValue
/// \brief local variable value
///
struct pdGDBFrameLocalVariableValue : public pdGDBData
{
public:
    ///////////////////////////////////////////////
    /// \brief Constructor
    ///
    /// \param[in]  variableName a name of requested local variable
    /// \param[in]  variableValue a ASCII presentation of local variable value
    pdGDBFrameLocalVariableValue(const gtString& variableName, const gtString&  variableValue);

    ///////////////////////////////////////////////
    /// \brief Get current instance type
    ///
    /// \return data type of the pdGDBFrameLocalVariableValue structure
    virtual pdGDBDataType type() const override;

    ///////////////////////////////////////////////
    /// \brief Standard virtual destructor
    ///
    virtual ~pdGDBFrameLocalVariableValue();

    gtString    _variableName;      ///< name of requested local variable
    gtString    _variableValue;     ///< ASCII presentation of local variable value

private:
    ///////////////////////////////////////////////
    /// \brief Standard constructor (unavailable)
    ///
    pdGDBFrameLocalVariableValue();
};

//////////////////////////////////////////////////////////////////////
/// \struct pdGDBFVariableType
/// \brief local variable value
///
struct pdGDBFVariableType : public pdGDBData
{
public:
    ///////////////////////////////////////////////
    /// \brief Constructor
    ///
    /// \param[in]  variableType a ASCII presentation of variable type
    pdGDBFVariableType(const gtString&  variableType);

    ///////////////////////////////////////////////
    /// \brief Get current instance type
    ///
    /// \return data type of the pdGDBFrameLocalVariableValue structure
    virtual pdGDBDataType type() const override;

    ///////////////////////////////////////////////
    /// \brief Standard virtual destructor
    ///
    virtual ~pdGDBFVariableType();

    gtString    _variableType;      ///< variable type

private:
    ///////////////////////////////////////////////
    /// \brief Standard constructor (unavailable)
    ///
    pdGDBFVariableType();
};


//////////////////////////////////////////////////////////////////////
/// \struct pdGDBBreakpointIndex
/// \brief gdb breakpoint index from gdb debugger
///
struct pdGDBBreakpointIndex : public pdGDBData
{
public:
    ///////////////////////////////////////////////
    /// \brief Standard constructor (unavailable)
    ///
    pdGDBBreakpointIndex() = delete;

    ///////////////////////////////////////////////
    /// \brief Constructor
    ///
    /// \param index a new gdb breakpoint index
    pdGDBBreakpointIndex(int index);

    ///////////////////////////////////////////////
    /// \brief Get current instance type
    ///
    /// \return data type of the pdGDBFrameLocalVariableValue structure
    virtual pdGDBDataType type() const override;

    ///////////////////////////////////////////////
    /// \brief Standard virtual destructor
    ///
    virtual ~pdGDBBreakpointIndex();

    int  m_gdbBreakpointIndex;      ///< gdb breakpoint index

private:
};

//////////////////////////////////////////////////////////////////////
/// \struct pdGDBHostStepErrorInfoIndex
/// \brief gdb error info on host step. At now it's meaningfull in case step-out from
///   the main frame where gdb return error and host application just will be
///   resumed for continue
///
struct pdGDBHostStepErrorInfoIndex : public pdGDBData
{
public:
    ///////////////////////////////////////////////
    /// \brief Standard constructor (unavailable)
    ///
    pdGDBHostStepErrorInfoIndex() = delete;

    ///////////////////////////////////////////////
    /// \brief Constructor
    ///
    /// \param errorInfo a gdb error string
    pdGDBHostStepErrorInfoIndex(const gtASCIIString& errorInfo);

    ///////////////////////////////////////////////
    /// \brief Get current instance type
    ///
    /// \return data type of the pdGDBFrameLocalVariableValue structure
    virtual pdGDBDataType type() const override;

    ///////////////////////////////////////////////
    /// \brief Standard virtual destructor
    ///
    virtual ~pdGDBHostStepErrorInfoIndex();

    gtASCIIString  m_gdbErrorInfo;      ///< gdb breakpoint index

private:
};


#endif  // __PDGDBDATASTRUCTS_H
