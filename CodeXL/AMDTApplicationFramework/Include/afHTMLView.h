//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afHTMLView.h
///
//==================================================================================

#ifndef __AFHTMLVIEW_H
#define __AFHTMLVIEW_H

// Qt:
#include <QWebView>

// Infra:
#include <AMDTOSWrappers/Include/osFilePath.h>

// Local:
#include <AMDTApplicationFramework/Include/afApplicationFrameworkDLLBuild.h>

// Forward declaration:

class AF_API afHTMLView : public QWebView
{
    Q_OBJECT

public:
    afHTMLView(const osFilePath& filePath);
    virtual ~afHTMLView();

    /// returns the file path
    osFilePath& FilePath() { return m_filePath; }

    /// reload the view
    void ReloadView();

protected slots:

    /// Slot handling the link clicked signal. Will open the links in an external window:
    virtual void OnLinkClicked(const QUrl& link);

private:
    /// handle the find source command link
    void FindSourceLink(int lineNumber);

    // the file path of the view
    osFilePath m_filePath;

};

#endif //__AFHTMLVIEW_H
