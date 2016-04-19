//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afStartupPage.h
///
//==================================================================================

#ifndef __AFSTARTUPPAGE_H
#define __AFSTARTUPPAGE_H

// Qt:
#include <QWebView>
#include <QWebPage>

// Infra:
#include <AMDTOSWrappers/Include/osFilePath.h>

// Local:
#include <AMDTApplicationFramework/Include/afApplicationFrameworkDLLBuild.h>

// Forward declaration:
class afApplicationCommands;

// ----------------------------------------------------------------------------------
// Class Name:          afWebPage: public QWebPage
// General Description: Inherit QWebPage. We need this class to override the JavaScript
//                      events since in the current HTML design, we need to use JS to
//                      execute CodeXL actions
// Author:              Sigal Algranaty
// Creation Date:       23/9/2014
// ----------------------------------------------------------------------------------
class afWebPage : public QWebPage
{
    Q_OBJECT
public:

    // Constructor:
    afWebPage(QObject* pParent = nullptr);

protected:

    // Overrides QWebPage: is used for catching requests from welcome page, and implement in Qt:
    virtual void javaScriptConsoleMessage(const QString& message, int lineNumber, const QString& sourceID);

    virtual bool acceptNavigationRequest(QWebFrame* frame, const QNetworkRequest& request, QWebPage::NavigationType type);


protected:

    // Application commands instance:
    afApplicationCommands* m_pApplicationCommands;
};

// ----------------------------------------------------------------------------------
// Class Name:          afStartupPage: public QWebView
// General Description: Inherit QWebView and is used for displaying the CodeXL HTML welcome page
// Author:              Sigal Algranaty
// Creation Date:       23/9/2014
// ----------------------------------------------------------------------------------
class AF_API afStartupPage : public QWebView
{
    Q_OBJECT

public:

    virtual ~afStartupPage();
    afStartupPage();

    // Update from current recent projects:
    bool UpdateHTML();

public slots:
    virtual void setSource(const QUrl& name);

protected:

    bool CanLinkBeClicked(const QUrl& url);

protected slots:

    /// Slot handling the link clicked signal. Will open the links in an external window:
    virtual void OnLinkClicked(const QUrl& link);

    /// Build the recently opened projects table, and replace it in the HTML text:
    /// \param htmlText the loaded HTML text (should contain dummy table for replacement)
    /// \return true for success (the text contain the expected table)
    bool BuildRecentlyOpenedProjectsTable(QString& htmlText);
};

#endif //__AFSTARTUPPAGE_H

