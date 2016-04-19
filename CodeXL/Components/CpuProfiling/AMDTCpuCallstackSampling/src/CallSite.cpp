//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CallSite.cpp
///
//==================================================================================

#include <CallSite.h>

bool CallSiteList::AddUnique(CallSite& site)
{
    bool ret = Find(site.m_traverseAddr) == end();

    if (ret)
    {
        InsertAfterHead(&site);
    }

    return ret;
}

CallSiteList::const_iterator CallSiteList::Find(gtVAddr traverseAddr) const
{
    const_iterator it = begin();

    for (const_iterator itEnd = end(); it != itEnd; ++it)
    {
        if (traverseAddr == (*it)->m_traverseAddr)
        {
            break;
        }
    }

    return it;
}

CallSiteList::iterator CallSiteList::Find(gtVAddr traverseAddr)
{
    iterator it = begin();

    for (iterator itEnd = end(); it != itEnd; ++it)
    {
        if (traverseAddr == (*it)->m_traverseAddr)
        {
            break;
        }
    }

    return it;
}
