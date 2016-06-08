//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file appMDIArea.h
///
//==================================================================================

#ifndef __APPMDIAREA_H
#define __APPMDIAREA_H

// Qt:
#include <QMdiArea>

class appMDIArea : public QMdiArea
{
    Q_OBJECT

public:

    appMDIArea();
    ~appMDIArea();


public Q_SLOTS:
    void closeActiveSubWindow();
};



#endif //__APPMDIAREA_H

