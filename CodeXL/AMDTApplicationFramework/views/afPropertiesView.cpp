//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afPropertiesView.cpp
///
//==================================================================================

// Qt
#include <AMDTApplicationComponents/Include/acQtIncludes.h>

// Infra
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// Local:
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afHTMLContent.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>
#include <AMDTApplicationFramework/Include/views/afPropertiesView.h>
#include <AMDTApplicationFramework/Include/views/afPropertiesUrlHandler.h>


#define AF_HTML_BG_COLOR QColor(125, 125, 125)

// ---------------------------------------------------------------------------
// Name:        afPropertiesView::afPropertiesView
// Description: Constructor.
// Arguments:   parent - My parent window.
// Author:      Yaki Tebeka
// Date:        1/11/2003
// ---------------------------------------------------------------------------
afPropertiesView::afPropertiesView(afProgressBarWrapper* pProgressBar, QWidget* pParent)
    : acQHTMLWindow(pParent), afBaseView(pProgressBar), m_pCurrentURLHandler(nullptr)
{
    // Set the background color:
    setTextBackgroundColor(AF_HTML_BG_COLOR);

    // Set an empty HTML string:
    setText(acGTStringToQString(afHTMLContent::emptyHTML()));

    // Set my initial message:
    gtString htmlPropertiesString;
    buildProjectNotLoadedMessage(htmlPropertiesString);
    setText(acGTStringToQString(htmlPropertiesString));
}

// ---------------------------------------------------------------------------
// Name:        afPropertiesView::~afPropertiesView
// Description: Destructor
// Author:      Yaki Tebeka
// Date:        1/11/2003
// ---------------------------------------------------------------------------
afPropertiesView::~afPropertiesView()
{
}

// ---------------------------------------------------------------------------
// Name:        afPropertiesView::clearView
// Description: Clears the properties view
// Author:      Eran Zinman
// Date:        15/5/2007
// ---------------------------------------------------------------------------
void afPropertiesView::clearView()
{
    gtString startDebuggingPropertiesPage;
    buildProcessStoppedMessage(startDebuggingPropertiesPage);
    setText(acGTStringToQString(startDebuggingPropertiesPage));
}

// ---------------------------------------------------------------------------
// Name:        gdInitializeApplicationCommand::setInitialMessage
// Description: Sets the properties view initial message.
// Author:      Yaki Tebeka
// Date:        20/8/2007
// ---------------------------------------------------------------------------
void afPropertiesView::setInitialMessage()
{
    // Get the current project name:
    gtString projectName = afProjectManager::instance().currentProjectSettings().projectName();

    // If no project is loaded:
    gtString htmlPropertiesString;

    if (projectName.isEmpty())
    {
        buildProjectNotLoadedMessage(htmlPropertiesString);
    }
    else
    {
        buildProcessStoppedMessage(htmlPropertiesString);
    }

    setText(acGTStringToQString(htmlPropertiesString));
}

// ---------------------------------------------------------------------------
// Name:        afPropertiesView::OnLinkClicked
// Description: Overrides afPropertiesView function
// Arguments:   QUrl& link
// Author:      Sigal Algranaty
// Date:        13/1/2011
// ---------------------------------------------------------------------------
void afPropertiesView::onLinkClicked(const QUrl& link)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pCurrentURLHandler != nullptr)
    {
        m_pCurrentURLHandler->handleURL(link);
    }
}

// ---------------------------------------------------------------------------
// Name:        afPropertiesView::buildProcessStoppedMessage
// Description: Builds a process stopped message
// Arguments:   gtString& propertiesHTMLMessage
// Author:      Sigal Algranaty
// Date:        31/12/2009
// ---------------------------------------------------------------------------
void afPropertiesView::buildProcessStoppedMessage(gtString& propertiesHTMLMessage)
{
    afHTMLContent htmlContent(AF_STR_PropertiesProcessNotRunning);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_NO_BG_LINE, AF_STR_PropertiesViewStartRunningComment);
    htmlContent.toString(propertiesHTMLMessage);
}

// ---------------------------------------------------------------------------
// Name:        afPropertiesView::buildProjectNotLoadedMessage
// Description: Build a project not loaded message
// Arguments:   gtString& propertiesHTMLMessage
// Author:      Sigal Algranaty
// Date:        26/3/2012
// ---------------------------------------------------------------------------
void afPropertiesView::buildProjectNotLoadedMessage(gtString& propertiesHTMLMessage)
{
    afHTMLContent htmlContent(AF_STR_PropertiesProjectNotLoaded);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_NO_BG_LINE, AF_STR_PropertiesViewLoadProjectComment);
    htmlContent.toString(propertiesHTMLMessage);
}

void afPropertiesView::setHTMLText(const QString& htmlText, afPropertiesUrlHandler* pURLHandler)
{
    // Must call clear method before set a new HTML text.
    // Reason: QTextBrowser chaching perviously images. If cached image was changed
    // set new HTML text will not affect without clear. Refresh/Update/Reload doesn't work.
    clear();

    // Set the HTML text:
    setText(htmlText);

    // Set the URL handler:
    m_pCurrentURLHandler = pURLHandler;
}
