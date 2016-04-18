//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afpropertiesurlhandler.h
///
//==================================================================================

#ifndef __AFPROPERTIESURLHANDLER_H
#define __AFPROPERTIESURLHANDLER_H

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afApplicationFrameworkDLLBuild.h>



// ----------------------------------------------------------------------------------
// Class Name:          AF_API afPropertiesView : public acQHTMLWindow, public afBaseView
// General Description: A view that displays the application framework HTML properties
//
// Author:              Sigal Algranaty
// Creation Date:       6/5/2012
// ----------------------------------------------------------------------------------
class AF_API afPropertiesUrlHandler
{
public:
    afPropertiesUrlHandler();
    virtual ~afPropertiesUrlHandler();

    virtual void handleURL(const QUrl& link) = 0;
};


#endif //__AFPROPERTIESURLHANDLER_H

