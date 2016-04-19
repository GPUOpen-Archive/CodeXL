//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afPropertiesView.h
///
//==================================================================================

#ifndef __AFPROPERTIESVIEW_H
#define __AFPROPERTIESVIEW_H

// Forward declaration:
class afPropertiesUrlHandler;

// Infra
#include <AMDTApplicationComponents/Include/acQHTMLWindow.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/views/afBaseView.h>
#include <AMDTApplicationFramework/Include/afApplicationFrameworkDLLBuild.h>



// ----------------------------------------------------------------------------------
// Class Name:          AF_API afPropertiesView : public acQHTMLWindow, public afBaseView
// General Description: A view that displays the application framework HTML properties
//
// Author:              Sigal Algranaty
// Creation Date:       6/5/2012
// ----------------------------------------------------------------------------------
class AF_API afPropertiesView : public acQHTMLWindow, public afBaseView
{
public:

    afPropertiesView(afProgressBarWrapper* pProgressBar, QWidget* pParent);
    virtual ~afPropertiesView();

    void clearView();
    void setInitialMessage();
    void setHTMLText(const QString& htmlText, afPropertiesUrlHandler* pURLHandler);

    // Edit menu commands
    virtual void onUpdateEdit_Copy(bool& isEnabled) {onUpdateEditCopy(isEnabled);}
    virtual void onUpdateEdit_SelectAll(bool& isEnabled) {isEnabled = true;}
    virtual void onUpdateEdit_Find(bool& isEnabled) {isEnabled = false;}
    virtual void onUpdateEdit_FindNext(bool& isEnabled) { isEnabled = false; }

    virtual void onEdit_Copy() {onEditCopy();}
    virtual void onEdit_SelectAll() {onEditSelectAll();}

protected slots:
    virtual void onLinkClicked(const QUrl& link);

protected:

    // Run time messages:
    void buildProcessStoppedMessage(gtString& propertiesHTMLMessage);
    void buildProjectNotLoadedMessage(gtString& propertiesHTMLMessage);

protected:

    afPropertiesUrlHandler* m_pCurrentURLHandler;

};


#endif //__AFPROPERTIESVIEW_H

