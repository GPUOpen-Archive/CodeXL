//=====================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/CLSummarizer.h $
/// \version $Revision: #8 $
/// \brief :  This file contains CLSummarizer
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/CLSummarizer.h#8 $
// Last checkin:   $DateTime: 2015/08/31 01:56:31 $
// Last edited by: $Author: salgrana $
// Change list:    $Change: 538740 $
//=====================================================================
#ifndef _CLSUMMARIZER_H_
#define _CLSUMMARIZER_H_

// Qt:
#include <QtCore>

#include "Session.h"

/// CL Summarizer class
class CLSummarizer
{
public:
    /// Initializes a new instance of the CLSummarizer class
    /// \param session API trace session
    CLSummarizer(TraceSession* session);

    /// Destructor
    ~CLSummarizer();

    /// Load summary pages from disk
    void CreateSummaryPages();

    /// Gets a value indicating whether or not a error warning page has been loaded
    /// \return True if warning is there
    bool GetHasErrorWarningPage() const { return m_hasErrorWarningPage; }

    /// Gets the map of summary pages, page name to page path
    /// \return the map of summary pages
    QMap<QString, QString> GetSummaryPagesMap() const { return m_summaryPagesMap; }

    /// Update the session folder after rename
    /// \param oldSessionDirectory - the session original folder
    /// \param newSessionDirectory - the session folder after the rename
    void UpdateRenamedSession(const osDirectory& oldSessionDirectory, const osDirectory& newSessionDirectory);

private:
    /// Disable copy constructor
    CLSummarizer(const CLSummarizer&);

    /// Disable assignment operator
    CLSummarizer& operator= (const CLSummarizer& dcc);

    /// Checks the specified file to see if it is the specified summary page type.  If so, it adds it to m_summaryPagesMap
    /// \param strFileExt the extension of the file being checked
    /// \param strFilePath the full path of the file being checked
    /// \param strWhichSummaryFile the summary file extension to check against
    /// \param strWhichSummaryTitle the title of the summary page, if found
    /// \return true if the specified summary page was found and added
    bool CheckAndAddSummaryPage(const QString& strFileExt, const QString& strFilePath, const QString& strWhichSummaryFile, const QString& strWhichSummaryTitle);

    QString                m_atpFile;                ///< full path to the atp file
    TraceSession*          m_pSession;               ///< API trace session
    bool                   m_summaryPagesLoaded;     ///< flag indicating whether or not the summary pages have been loaded
    bool                   m_hasErrorWarningPage;    ///< flag indicating whether or not a error warning page has been loaded
    QMap<QString, QString> m_summaryPagesMap;        ///< map of summary page name to path of file
};


#endif // _CLSUMMARIZER_H_

