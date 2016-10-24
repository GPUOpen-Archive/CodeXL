//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This file declares a base class for tracing HSA API
//==============================================================================

#ifndef _HSA_API_BASE_H_
#define _HSA_API_BASE_H_

#include "../Common/APIInfoManagerBase.h"
#include "../Common/APITraceUtils.h"
#include "../HSAFdnCommon/HSAFunctionDefs.h"

//------------------------------------------------------------------------------------
/// HSAAPI base class
//------------------------------------------------------------------------------------
class HSAAPIBase : public APIBase
{
public:
    /// Constructor
    HSAAPIBase();

    /// Virtual destructor
    virtual ~HSAAPIBase();

    /// Write API entry
    /// \param sout output stream
    void WriteAPIEntry(std::ostream& sout);

    /// Write stack entry
    /// \param sout output stream
    void WriteStackEntry(std::ostream& sout);

    /// Write timestamp entry
    /// \param sout output stream
    /// \param bTimeout a flag indicating output mode
    /// \return True if timestamps are ready
    bool WriteTimestampEntry(std::ostream& sout, bool bTimeout);

public:
    HSA_API_Type m_type;       ///< api type enum

private:
    /// Disable copy constructor
    /// \param[in] obj  the input object
    HSAAPIBase(const HSAAPIBase& obj);

    /// Disable assignment operator
    /// \param[in] obj  the input object
    /// \return a reference of the object
    HSAAPIBase& operator=(const HSAAPIBase& obj);

    /// Creates the stack entry (on-demand)
    void CreateStackEntry();
};

#endif // _HSA_API_BASE_H_
