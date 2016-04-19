//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afQtViewCreatorAbstract.h
///
//==================================================================================

#ifndef __AFQTVIEWCREATORABSTRACT_H
#define __AFQTVIEWCREATORABSTRACT_H

// Qt:
#include <QWidget>

// Infra:
#include <AMDTApplicationFramework/Include/afApplicationFrameworkDLLBuild.h>
#include <AMDTApplicationFramework/Include/afViewCreatorAbstract.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtVector.h>

// ----------------------------------------------------------------------------------
// Class Name:          afQtViewCreatorAbstract
// General Description: abstract class for the view creator
//                      A view creator can create several views. using the API to see
//                      how many views can be create.
// Author:              Sigal Algranaty
// Creation Date:       2/8/2011
// ----------------------------------------------------------------------------------
class AF_API afQtViewCreatorAbstract : public afViewCreatorAbstract
{
public:
    afQtViewCreatorAbstract();
    virtual ~afQtViewCreatorAbstract();

    // Initialize the creator:
    virtual void initialize();

    // Get the specific window:
    virtual QWidget* widget(int viewIndex);

    // Get the specific window:
    virtual bool wasWidgetCreated(QWidget* pWidget);

    // Get number of views that was created by this creator:
    virtual int amountOfCreatedViews() {return (int)m_viewsCreated.size();};

    // clear the minimal size
    virtual void restoreMinimalSize();


protected:
    // The Qt windows handled by the creator:
    gtVector<QWidget*> m_viewsCreated;

};

#endif // __AFQTVIEWCREATORABSTRACT_H
