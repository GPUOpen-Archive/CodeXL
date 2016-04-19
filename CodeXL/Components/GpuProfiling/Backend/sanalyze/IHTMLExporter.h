//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  HTML exporter interface
//==============================================================================

#ifndef _I_HTML_EXPORTER_H_
#define _I_HTML_EXPORTER_H_

#include <iostream>

//------------------------------------------------------------------------------------
/// Abstract class
/// HTML Exporter
//------------------------------------------------------------------------------------
class IHTMLExporter
{
public:
    /// Pure virtual function
    /// \param sout output stream
    virtual void ExportToHTML(std::ostream& sout) = 0;
};

#endif //_I_HTML_EXPORTER_H_
