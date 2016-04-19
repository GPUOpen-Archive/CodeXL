//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CPUProfileDataTableItem.cpp
///
//==================================================================================

/// Local:
#include <inc/CPUProfileDataTableItem.h>

CPUProfileDataTableItem::SortOrder CPUProfileDataTableItem::GetTableSortOrder(Qt::SortOrder order)
{
    SortOrder sortOrder = UNKNOWN_ORDER;

    switch (order)
    {
        case Qt::AscendingOrder:
            sortOrder = ASCENDING_ORDER;
            break;

        case Qt::DescendingOrder:
            sortOrder = DESCENDING_ORDER;
            break;
    }

    return sortOrder;
}

bool CPUProfileDataTableItem::operator<(const QTableWidgetItem& other) const
{
    bool retVAl = false;

    SortOrder itemSortOrder = static_cast<SortOrder>(qvariant_cast<int>(data(AC_USER_ROLE_OTHER_ROW)));

    // if left value is of item from "other" row - return always that it is the last
    if (itemSortOrder != UNKNOWN_ORDER)
    {
        retVAl = (itemSortOrder == ASCENDING_ORDER) ? false : true;
    }
    else
    {
        // if right value is of item from "other" row - return always that it is the last
        itemSortOrder = static_cast<SortOrder>(qvariant_cast<int>(other.data(AC_USER_ROLE_OTHER_ROW)));

        if (itemSortOrder != UNKNOWN_ORDER)
        {
            retVAl = (itemSortOrder == ASCENDING_ORDER) ? true : false;
        }
        else
        {
            // if both sides are regular items - use original operator<
            retVAl = acTableWidgetItem::operator<(other);
        }
    }

    return retVAl;
}