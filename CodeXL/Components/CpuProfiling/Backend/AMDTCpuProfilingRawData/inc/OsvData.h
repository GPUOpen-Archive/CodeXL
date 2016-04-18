//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file OsvData.h
///
//==================================================================================

#ifndef _OSVDATA_H_
#define _OSVDATA_H_

#include "CpuProfilingRawDataDLLBuild.h"
#include <qstring.h>

#define OSV_VERSION  1

class CP_RAWDATA_API OsvDataItem
{
public:
    OsvDataItem();
    OsvDataItem(QString hdr, QString val = "", QString lnk = "");
    ~OsvDataItem();

    OsvDataItem* addSibling(QString header, QString value = "", QString link = "");
    OsvDataItem* addChild(QString header, QString value = "", QString link = "");
    void appendChild(OsvDataItem* pChild);
    void clear();

    OsvDataItem* sibling;
    OsvDataItem* child;
    QString header;
    QString value;
    QString link;
};

#endif // _OSVDATA_H_
