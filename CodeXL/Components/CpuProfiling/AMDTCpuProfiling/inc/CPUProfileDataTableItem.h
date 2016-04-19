//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CPUProfileDataTableItem.h
///
//==================================================================================

#ifndef __CPUPROFILEDATATABLEITEM_H
#define __CPUPROFILEDATATABLEITEM_H

// Infra:
#include <AMDTApplicationComponents/Include/acTableWidgetItem.h>
#include <AMDTApplicationComponents/Include/acUserRoles.h>


class CPUProfileDataTableItem : public acTableWidgetItem
{
public:

    enum SortOrder
    {
        UNKNOWN_ORDER,
        ASCENDING_ORDER,
        DESCENDING_ORDER
    };

    // constructors
    CPUProfileDataTableItem(int type = Type) : acTableWidgetItem(type) {}
    explicit CPUProfileDataTableItem(const QString& text, int type = Type) : acTableWidgetItem(text, type) {}
    explicit CPUProfileDataTableItem(const QIcon& icon, const QString& text, int type = Type) : acTableWidgetItem(icon, text, type) {}

    // this dunction converts the sort order value from Qt::SortOrder type to CPUProfileDataTableItem::SortOrder type
    static SortOrder GetTableSortOrder(Qt::SortOrder order);

    // override operator< specific for CPUProfileDataTableItem items
    // this operator handles differently items from "othr" row (6th row from top 5 table - hotspot)
    bool operator<(const QTableWidgetItem& other) const;
};
#endif //__CPUPROFILEDATATABLEITEM_H

