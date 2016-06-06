//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acSplitter.h
///
//==================================================================================

//------------------------------ acTabWidget.h ------------------------------

#ifndef __ACSPLITTER_H
#define __ACSPLITTER_H

// Qt:
#include <QSplitter>


// Local:
#include <AMDTApplicationComponents/Include/acApplicationComponentsDLLBuild.h>


// ----------------------------------------------------------------------------------
// Class Name:          AC_API acSplitter : public QSplitter
// General Description: Overriding QSplitter class in order to access its protected
//                      functionality
// Author:              Sigal Algranaty
// Creation Date:       4/1/2014
// ----------------------------------------------------------------------------------
class AC_API acSplitter : public QSplitter
{
    Q_OBJECT
public:

    acSplitter(QWidget* pParent = NULL);
    acSplitter(Qt::Orientation orientation, QWidget* pParent = NULL);
    ~acSplitter() {};

    /// This function is simply accessing moveSplitter, which is a protected function in QSplitter:
    void MoveSplitter(int index, int position);

};

#endif //__ACSPLITTER_H

