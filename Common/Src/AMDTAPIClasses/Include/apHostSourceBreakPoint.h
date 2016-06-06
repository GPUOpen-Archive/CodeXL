//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apHostSourceBreakPoint.h
///
//==================================================================================

//------------------------------ apHostSourceCodeBreakpoint.h ------------------------------
/// \file apHostSourceCodeBreakpoint.h
/// \brief Host code source (*.cpp, *.hpp etc) breakpoint defenition structure
/// \author Vadim Entov
/// \date 12/09/2015

#ifndef __APHOSTSOURCECODEBREAKPOINT_H
#define __APHOSTSOURCECODEBREAKPOINT_H

// Infra:
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSAPIWrappers/Include/oaOSAPIDefinitions.h>

// Local:
#include <AMDTAPIClasses/Include/apAPIClassesDLLBuild.h>
#include <AMDTAPIClasses/Include/apBreakPoint.h>


/////////////////////////////////////////////////////////////////////////////
/// \class apHostSourceCodeBreakpoint
/// \brief Host code source (*.cpp, *.hpp etc) breakpoint defenition structure
/// \author Vadim Entov
/// \date 12/09/2015
class AP_API apHostSourceCodeBreakpoint : public apBreakPoint
{
public:
    /////////////////////////////////////////////////////////////////////////////
    /// \brief Default constructor
    ///
    /// \author Vadim Entov
    /// \date 12/09/2015
    apHostSourceCodeBreakpoint() {}

    /////////////////////////////////////////////////////////////////////////////
    /// \brief Additional constructor
    /// \param filePath a path for targen breakpoint file
    /// \param lineNum a breakpoint line number
    ///
    /// \author Vadim Entov
    /// \date 12/09/2015
    apHostSourceCodeBreakpoint(const osFilePath& filePath, int lineNum = -1);

    /////////////////////////////////////////////////////////////////////////////
    /// \brief Destructor
    /// \author Vadim Entov
    /// \date 12/09/2015
    ~apHostSourceCodeBreakpoint();

    /////////////////////////////////////////////////////////////////////////////
    /// \brief Other breakpoint compare.
    ///
    /// \param otherBreakpoint a reference to comaparing breakpoint class instance
    ///
    /// \return In case self and target the same - true
    ///
    /// \author Vadim Entov
    /// \date 12/09/2015
    virtual bool compareToOther(const apBreakPoint& otherBreakpoint) const; // override;

    // Overrides osTransferableObject:

    /////////////////////////////////////////////////////////////////////////////
    /// \brief Get current class instance type
    ///
    /// \return instance type
    ///
    /// \author Vadim Entov
    /// \date 12/09/2015
    virtual osTransferableObjectType type() const override;

    /////////////////////////////////////////////////////////////////////////////
    /// \brief Serialization to channel
    ///
    /// \param ipcChannel a reference to ICP channel instance
    ///
    /// \return true on function call succedes or false
    ///
    /// \author Vadim Entov
    /// \date 12/09/2015
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const override;

    /////////////////////////////////////////////////////////////////////////////
    /// \brief DeSerialization from channel
    ///
    /// \param ipcChannel a reference to ICP channel instance
    ///
    /// \return true on function call succedes or false
    ///
    /// \author Vadim Entov
    /// \date 12/09/2015
    virtual bool readSelfFromChannel(osChannel& ipcChannel) override;

    // Accessors:

    /////////////////////////////////////////////////////////////////////////////
    /// \brief Set breakpoint line number
    ///
    /// \param lineNum a breakpoint line number
    ///
    /// \author Vadim Entov
    /// \date 12/09/2015
    void setLineNumber(int lineNum) { m_lineNumber = lineNum; };

    /////////////////////////////////////////////////////////////////////////////
    /// \brief Get breakpoint line number
    ///
    /// \return breakpoint line number
    ///
    /// \author Vadim Entov
    /// \date 12/09/2015
    int lineNumber() const { return m_lineNumber; }

    // Breakpoint resolution:
    void setFilePath(const osFilePath& path) { m_FilePath = path; }

    const osFilePath& filePath() const { return m_FilePath; }

private:
    osFilePath m_FilePath; ///< breakpoint file path
    int        m_lineNumber;                 ///< breakpoiint line number
};

#endif //__APKERNELSOURCECODEBREAKPOINT_H

