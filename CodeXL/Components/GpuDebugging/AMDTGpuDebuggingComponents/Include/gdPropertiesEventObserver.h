//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdPropertiesEventObserver.h
///
//==================================================================================

//------------------------------ gdPropertiesEventObserver.h ------------------------------

#ifndef __GDPROPERTIESEVENTOBSERVER_H
#define __GDPROPERTIESEVENTOBSERVER_H

// Ignore warnings:
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afHTMLContent.h>

// Qt:
#include <QTextBrowser>


// Forward declaration:
class afApplicationCommands;
class afPropertiesView;
class gdApplicationCommands;
class apMemoryLeakEvent;
class gdDebugApplicationTreeData;

// Infra
#include <AMDTApplicationComponents/Include/acQHTMLWindow.h>
#include <AMDTAPIClasses/Include/Events/apIEventsObserver.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/views/afBaseView.h>
#include <AMDTApplicationFramework/Include/afPropertiesUrlHandler.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerAppDLLBuild.h>

// ----------------------------------------------------------------------------------
// Class Name:           gdPropertiesEventObserver: public acWXHTMLWindow
// General Description:
//  A view that describe in more details whats happening in other views
//
// Author:               Yaki Tebeka
// Creation Date:        1/11/2003
// ----------------------------------------------------------------------------------
class GD_API gdPropertiesEventObserver: public apIEventsObserver, public afPropertiesUrlHandler
{
public:
    static gdPropertiesEventObserver& instance();
    virtual ~gdPropertiesEventObserver();

    void clearView();

    virtual void handleURL(const QUrl& link);

    // wxWidgets events handling:
    void onInit();

    // Overrides apIEventsObserver:
    virtual const wchar_t* eventObserverName() const { return L"PropertiesView"; };

    virtual bool displayItemProperties(const afApplicationTreeItemData* pItemData, bool displayItemChildren, bool displayThumbnail, bool displayExtendedInformation);
    virtual bool BuildItemHTMLProperties(const afApplicationTreeItemData* pItemData, bool displayItemChildren, bool displayThumbnail, bool displayExtendedInformation, afHTMLContent& htmlContent);
    void setPropertiesFromText(const QString& htmlText);

private:
    // Overrides apIEventsObserver:
    virtual void onEvent(const apEvent& eve, bool& vetoEvent);

    void onBreakpointHit(const apEvent& eve);

    // Build process stop string for stand alone and package:
    virtual void buildProcessStopString(afHTMLContent& htmlContent);

    // Build project not loaded string for stand alone and package:
    virtual void buildProjectNotLoadedString(gtString& propertiesInfo);

    // Display a system information item:
    bool displaySystemInformationItem(const afApplicationTreeItemData& objectID);

    // Set internal members info:
    void setPropertiesViewInfo();

private:
    friend class gdSingletonsDelete;

    // Static single instance:
    static gdPropertiesEventObserver* m_spMySingleInstance;

    // Do not allow the use of my constructor:
    gdPropertiesEventObserver();

    // Holds the last memory leak event handled:
    apMemoryLeakEvent* _pLastMemoryLeakEvent;

    // Application commands instance:
    afApplicationCommands* m_pApplicationCommands;
    gdApplicationCommands* m_pGDApplicationCommands;

    // Holds the framework properties view:
    afPropertiesView* m_pPropertiesView;
    afProgressBarWrapper* m_pProgressBar;

};


#endif //__GDPROPERTIESEVENTOBSERVER_H

