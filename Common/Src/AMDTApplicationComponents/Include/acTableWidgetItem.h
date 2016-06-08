//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acTableWidgetItem.h
///
//==================================================================================

//------------------------------ acTableWidgetItem.h ------------------------------

#ifndef __ACTABLEWIDGETITEM
#define __ACTABLEWIDGETITEM

// QT:
#include <QTableWidgetItem>

// Same behavior as QTableWidgetItem but have case-insensitive sort policy.
class acTableWidgetItem : public QTableWidgetItem
{
public:
    acTableWidgetItem(int type = Type) : QTableWidgetItem(type) {}
    explicit acTableWidgetItem(const QString& text, int type = Type) : QTableWidgetItem(text, type) {}
    explicit acTableWidgetItem(const QIcon& icon, const QString& text, int type = Type) : QTableWidgetItem(icon, text, type) {}

    virtual bool operator<(const QTableWidgetItem& other) const
    {
        const QVariant v1 = data(Qt::DisplayRole), v2 = other.data(Qt::DisplayRole);

        if (v1.userType() == QVariant::String && v2.userType() == QVariant::String)
        {
            return v1.toString().compare(v2.toString(), Qt::CaseInsensitive) < 0;
        }

        return this->QTableWidgetItem::operator<(other);
    }
};

#endif  // __ACTABLEWIDGETITEM
