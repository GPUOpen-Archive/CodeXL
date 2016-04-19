//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file OsvData.cpp
///
//==================================================================================

#include <OsvData.h>


OsvDataItem::OsvDataItem()
{
    sibling = NULL;
    child = NULL;
}

OsvDataItem::OsvDataItem(QString hdr, QString val, QString lnk)
{
    sibling = NULL;
    child = NULL;
    this->header = hdr;
    this->value = val;
    this->link = lnk;
}

OsvDataItem::~OsvDataItem()
{
    clear();
}

OsvDataItem* OsvDataItem::addSibling(QString headerIn, QString valueIn, QString linkIn)
{
    if (sibling)
    {
        delete sibling;
        sibling = NULL;
    }

    sibling = new OsvDataItem(headerIn, valueIn, linkIn);
    return sibling;
}


OsvDataItem* OsvDataItem::addChild(QString headerIn, QString valueIn, QString linkIn)
{
    if (child)
    {
        delete child;
        child = NULL;
    }

    child = new OsvDataItem(headerIn, valueIn, linkIn);
    return child;
}

void OsvDataItem::appendChild(OsvDataItem* pChild)
{
    if (!pChild)
    {
        return;
    }

    if (!child)
    {
        child = pChild;
    }
    else
    {
        OsvDataItem* pTmp = child;

        while (pTmp->sibling != NULL)
        {
            pTmp = pTmp->sibling;
        }

        pTmp->sibling = pChild;
    }
}


void OsvDataItem::clear()
{
    if (child)
    {
        child->clear();
        delete child;
        child = NULL;
    }

    if (sibling)
    {
        sibling->clear();
        delete sibling;
        sibling = NULL;
    }
}
