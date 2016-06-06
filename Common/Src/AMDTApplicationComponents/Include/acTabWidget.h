//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acTabWidget.h
///
//==================================================================================

//------------------------------ acTabWidget.h ------------------------------

#ifndef __ACTABWIDGET_H
#define __ACTABWIDGET_H

// Qt:
#include <QTableWidget>


// Local:
#include <AMDTApplicationComponents/Include/acApplicationComponentsDLLBuild.h>


// ----------------------------------------------------------------------------------
// Class Name:          AC_API acTabWidget : public QTabWidget
// General Description: Implementation for the Qt QTabWidget object.
// Author:              Sigal Algranaty
// Creation Date:       12/6/2012
// ----------------------------------------------------------------------------------
class AC_API acTabWidget : public QTabWidget
{
    Q_OBJECT
public:

    acTabWidget(QWidget* pParent = NULL);
    virtual ~acTabWidget();

    void setTabBarVisible(bool isVisible);

};



#endif //__ACTABWIDGET_H

