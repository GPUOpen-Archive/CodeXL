//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Basic dictionary class
//==============================================================================

#ifndef DICTIONARY_H
#define DICTIONARY_H

#include "xml.h"
#include "misc.h"
#include <map>

/// Basically a map with a few helpers
template <class tKey, class tVal> class Dictionary
{
    /// hash
    std::map<tKey , tVal> DataBase;
public:

    /// returns the number of elements in the dictionary
    size_t size()
    {
        return DataBase.size();
    }

    /// add a key, value tuple
    void Add(tKey k, tVal v)
    {
        DataBase[k] = v;
    }

    /// no idea
    bool Find(tKey k, tVal& vout)
    {
        typename std::map<tKey , tVal>::iterator i;

        i = DataBase.find(k);

        if (i == DataBase.end())
        {
            return false;
        }
        else
        {
            vout = i->second;
            return true;
        }
    }

    /// returns a string with all the keys
    gtASCIIString DumpKeys()
    {
        gtASCIIString keys;
        typename std::map<tKey , tVal>::iterator i;

        for (i = DataBase.begin(); i != DataBase.end(); i++)
        {
            keys += XML("key", FormatText("0x%p", i->first).asCharArray());
        }

        return XML("Dictionary", keys.asCharArray());
    }
};

#endif // DICTIONARY_H
