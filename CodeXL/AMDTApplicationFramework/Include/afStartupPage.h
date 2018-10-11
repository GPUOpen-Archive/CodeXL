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
#include <QWebEngineView>
#include <QWebEnginePage>

// Infra:
#include <AMDTOSWrappers/Include/osFilePath.h>

// Local:
#include <AMDTApplicationFramework/Include/afApplicationFrameworkDLLBuild.h>

// Forward declaration:
class afApplicationCommands;

// ----------------------------------------------------------------------------------
// Class Name:          afWebPage: public QWebEnginePage
// General Description: Inherit QWebEnginePage. We need this class to override the JavaScript
//                      events since in the current HTML design, we need to use JS to
//                      execute CodeXL actions
// Author:              Sigal Algranaty
// Creation Date:       23/9/2014
// ----------------------------------------------------------------------------------
class afWebPage : public QWebEnginePage
{
    Q_OBJECT
public:

    // Constructor:
    afWebPage(QObject* pParent = nullptr);

protected:

    // Overrides QWebEnginePage: is used for catching requests from welcome page, and implement in Qt:
    virtual void javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level, const QString& message, int lineNumber, const QString& sourceID);

    // Overrides QWebEnginePage: is used to accept navigation request from welcom page
    virtual bool acceptNavigationRequest(const QUrl &url, NavigationType type, bool isMainFrame);

signals:

    void linkClicked(const QUrl&);

protected:

    // Application commands instance:
    afApplicationCommands* m_pApplicationCommands;
};

// ----------------------------------------------------------------------------------
// Class Name:          afStartupPage: public QWebEngineView
// General Description: Inherit QWebEngineView and is used for displaying the CodeXL HTML welcome page
// Author:              Sigal Algranaty
// Creation Date:       23/9/2014
// ----------------------------------------------------------------------------------
class AF_API afStartupPage : public QWebEngineView
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
    afWebPage* m_pPage;

protected slots:

    /// Slot handling the link clicked signal. Will open the links in an external window:
    virtual void OnLinkClicked(const QUrl& link);

    /// Build the recently opened projects table, and replace it in the HTML text:
    /// \param htmlText the loaded HTML text (should contain dummy table for replacement)
    /// \return true for success (the text contain the expected table)
    bool BuildRecentlyOpenedProjectsTable(QString& htmlText);
};

#endif //__AFSTARTUPPAGE_H

