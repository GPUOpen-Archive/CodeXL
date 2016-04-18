//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This is an interface for a parser progress monitor.
//==============================================================================

#ifndef _I_PARSER_PROGRESS_MONITOR_H_
#define _I_PARSER_PROGRESS_MONITOR_H_

//------------------------------------------------------------------------------------
/// Abstract class
/// Parser Progress Monitor
//------------------------------------------------------------------------------------
class IParserProgressMonitor
{
public:
    /// Pure virtual function
    /// \param strProgressMessage the progress message to display
    /// \param uiCurItem the index of the current item being parsed
    /// \param uiTotalItems the total numbers of items to be parsed
    virtual void OnParserProgress(const std::string& strProgressMessage, unsigned int uiCurItem, unsigned int uiTotalItems) = 0;

    /// Virtual destructor
    virtual ~IParserProgressMonitor() {}
};

#endif
